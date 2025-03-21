#include <iostream>
#include <windows.h>
#include <mutex>
#include <conio.h> // For _kbhit() and _getch()
#include <cstdlib> // For rand()
#include "SharedHeader.h"

using namespace std;

namespace mygame
{
    int power = 50;
    int score = 0;
    char gameMap[10][20];
    int playerX = 9, playerY = 10;
    mutex mtx;
    bool gameRunning = true;
    string eventMessage = "";
    int messageTimer = 0;

    // Helper function to set console cursor position
    void SetCursorPosition(int x, int y) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(hConsole, pos);
    }

    // Draw the static background (borders and instructions) once
    void DrawBackground() {
        system("cls"); // Clear screen once at the start
        // Draw top border
        cout << "+--------------------+\n";

        // Draw game area with side borders
        for (int i = 0; i < 10; i++) {
            cout << "|                    |\n";
        }

        // Draw bottom border
        cout << "+--------------------+\n";

        // Display static game info
        cout << "Power: " << power << " | Score: " << score << "\n";
        cout << "Move: 'A', 'D' | Avoid X | Catch + or * | Press 'Q' to Quit\n";
        cout << "+ = Heal (20) | * = Speed (5) | X = Enemy (-10)\n";
        cout << "Event: \n"; // Placeholder for event messages
    }

    // Draw only the dynamic parts (game map, power, score, and event message)
    void DrawGame() {
        lock_guard<mutex> lock(mtx);
        // Update the game map (rows 1 to 10 in console)
        for (int i = 0; i < 10; i++) {
            SetCursorPosition(1, i + 1);
            for (int j = 0; j < 20; j++) {
                if (gameMap[i][j] == 'P') cout << "P";  // Player
                else if (gameMap[i][j] == 'E') cout << "X"; // Enemy
                else if (gameMap[i][j] == 'H') cout << "+"; // Healing
                else if (gameMap[i][j] == 'S') cout << "*"; // Speed Boost
                else cout << " "; // Empty space
            }
        }

        // Update power and score display (row 12 in console)
        SetCursorPosition(0, 12);
        cout << "Power: " << power << " | Score: " << score << "   ";

        // Update event message (row 15 in console)
        SetCursorPosition(0, 15);
        if (messageTimer > 0) {
            cout << "Event: " << eventMessage << "    ";
            messageTimer--;
        }
        else {
            cout << "Event:               ";
        }
    }

    // Initialize the game grid and player position
    void InitializeGame() {
        lock_guard<mutex> lock(mtx);
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                gameMap[i][j] = ' ';
        gameMap[playerX][playerY] = 'P';
        score = 0;
        power = 50;
    }

    // Thread to handle player movement based on keyboard input
    void MovePlayer() {
        while (gameRunning) {
            if (_kbhit()) {
                char ch = _getch();
                {
                    lock_guard<mutex> lock(mtx);
                    ch = toupper(ch);
                    gameMap[playerX][playerY] = ' ';
                    if (ch == 'A' && playerY > 0) playerY--;  // Move left
                    if (ch == 'D' && playerY < 19) playerY++; // Move right
                    if (ch == 'Q') gameRunning = false;       // Quit game
                    gameMap[playerX][playerY] = 'P';
                }
            }
            Sleep(50);
        }
    }

    // Thread to spawn enemies ('E'), healing items ('H'), and speed items ('S')
    void SpawnObjects() {
        int spawnInterval = 2000; // Start with 2 seconds
        while (gameRunning) {
            Sleep(spawnInterval);
            lock_guard<mutex> lock(mtx);
            int y = rand() % 20;
            int objectType = rand() % 3; // 0: Enemy, 1: Healing, 2: Speed
            if (gameMap[0][y] == ' ') {
                if (objectType == 0) gameMap[0][y] = 'E';      // Enemy
                else if (objectType == 1) gameMap[0][y] = 'H'; // Healing
                else gameMap[0][y] = 'S';                      // Speed
            }
            // Gradually decrease spawn interval to increase difficulty
            if (spawnInterval > 1000) {
                spawnInterval -= 50;
            }
        }
    }

    // Thread to move objects downward and handle collisions
    void MoveObjects() {
        while (gameRunning) {
            Sleep(500);
            {
                lock_guard<mutex> lock(mtx);
                for (int i = 8; i >= 0; i--) {
                    for (int j = 0; j < 20; j++) {
                        if (gameMap[i][j] == 'E' || gameMap[i][j] == 'H' || gameMap[i][j] == 'S') {
                            if (i + 1 < 10) {
                                if (gameMap[i + 1][j] == 'P') { // Collision with player
                                    if (gameMap[i][j] == 'E') {
                                        power -= 10; // Enemy hit
                                        score -= 5;  // Penalty for hitting enemy
                                        eventMessage = "Hit by Enemy! -10";
                                        messageTimer = 10;
                                        Beep(300, 300); 
                                        if (power <= 0) gameRunning = false;
                                    }
                                    else if (gameMap[i][j] == 'H') {
                                        power = min(100, power + 20); // Healing
                                        score += 10; // Reward for collecting healing
                                        eventMessage = "Healed! +20";
                                        messageTimer = 10;
                                        Beep(800, 200); // High-pitched tone for healing
                                    }
                                    else {
                                        power += 5; // Speed item gives small power boost
                                        score += 5;  // Reward for collecting speed
                                        eventMessage = "Speed Boost! +5";
                                        messageTimer = 10;
                                        Beep(600, 200);
                                    }
                                    // Object disappears on collision, but P stays
                                }
                                else {
                                    gameMap[i + 1][j] = gameMap[i][j]; // Move object downward
                                }
                            }
                            gameMap[i][j] = ' '; // Clear previous position
                        }
                    }
                }
                // Remove 'E', 'H', and 'S' when they reach the ground (row 9)
                for (int j = 0; j < 20; j++) {
                    if (gameMap[9][j] == 'E' || gameMap[9][j] == 'H' || gameMap[9][j] == 'S') {
                        if (gameMap[9][j] == 'E') {
                            eventMessage = "Enemy Escaped!";
                            messageTimer = 10;
                            Beep(400, 100); 
                            Beep(300, 100);
                        }
                        gameMap[9][j] = ' ';
                    }
                }
                // Ensure player remains on the grid
                gameMap[playerX][playerY] = 'P';
            }
        }
    }

    // Thread to reduce power over time and increase score for survival
    void PowerDecay() {
        int decayRate = 2; // Start with 2 power loss every 5 seconds
        while (gameRunning) {
            Sleep(5000);
            lock_guard<mutex> lock(mtx);
            power -= decayRate;
            score += 2; // Reward for surviving
            if (power <= 0) gameRunning = false;
            // Increase decay rate over time to increase difficulty
            if (decayRate < 5) {
                decayRate++;
            }
        }
    }

    // Thread to update and draw the game state
    void GameLoop() {
        DrawBackground(); // Draw static parts once at the start
        while (gameRunning) {
            Sleep(100);
            DrawGame(); // Update only the dynamic parts
        }
    }
}