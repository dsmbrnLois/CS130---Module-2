#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <windows.h>
#include <conio.h>
#include <mutex>
#include "SharedHeader.h"

using namespace std;

namespace mygame {
    // Constants for game
    const int MAP_HEIGHT = 10;
    const int MAP_WIDTH = 20;
    const int INITIAL_SPAWN_INTERVAL = 2000; 
    const int MIN_SPAWN_INTERVAL = 1000;     
    const int INITIAL_ENEMY_SPAWN_CHANCE = 33; 
    const int MAX_ENEMY_SPAWN_CHANCE = 60;   
    const int INITIAL_POWER = 100;
    const int INITIAL_DECAY_RATE = 5;
    const int MAX_DECAY_RATE = 10;
    const int DECAY_INTERVAL = 2000;
    const int FRAME_RATE = 100;      

    // Game class to encapsulate game state and logic
    class Game {
    private:
        bool gameRunning;
        int playerX, playerY;
        int power;
        int score;
        char gameMap[MAP_HEIGHT][MAP_WIDTH];
        string eventMessage;
        int messageTimer;
        mutex mtx;

        struct PlayerScore {
            string name;
            int score;
        };
        vector<PlayerScore> leaderboard;

        void LoadLeaderboard() {
            ifstream file("leaderboard.txt");
            leaderboard.clear();
            string name;
            int score;
            while (file >> name >> score) {
                leaderboard.push_back({ name, score });
            }
            file.close();
            sort(leaderboard.begin(), leaderboard.end(), [](const PlayerScore& a, const PlayerScore& b) {
                return a.score > b.score;
                });
        }

        void ShowLeaderboard() {
            LoadLeaderboard();
            system("cls");
            cout << "=== Leaderboard ===\n";
            for (size_t i = 0; i < leaderboard.size() && i < 10; i++) {
                cout << i + 1 << ". " << leaderboard[i].name << " - " << leaderboard[i].score << "\n";
            }
            cout << "\nPress any key to return to the menu...";
            cin.ignore();
            cin.get();
        }

        void SetCursorPosition(int x, int y) {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
            SetConsoleCursorPosition(hConsole, pos);
        }

        void DrawStaticBackground() {
            system("cls"); // Clear screen once at the start
            cout << "| A/D to Move | Collect: + for score, * for healing | Avoid: X | Quit: Q |\n";
            cout << "----------------------\n";
            for (int i = 0; i < MAP_HEIGHT; i++) {
                cout << "|                    |\n";
            }
            cout << "----------------------\n";
            cout << "Power: " << power << " | Score: " << score << "\n";
            cout << "Event: \n";
        }

        void DrawDynamicElements() {
            lock_guard<mutex> lock(mtx);
            // Update the game map (rows 2 to 11 in console)
            for (int i = 0; i < MAP_HEIGHT; i++) {
                SetCursorPosition(1, i + 2);
                for (int j = 0; j < MAP_WIDTH; j++) {
                    if (gameMap[i][j] == 'P') cout << "P";  // Player
                    else if (gameMap[i][j] == 'X') cout << "X"; // Enemy
                    else if (gameMap[i][j] == '*') cout << "*"; // Healing
                    else if (gameMap[i][j] == '+') cout << "+"; // Score Boost
                    else cout << " "; // Empty space
                }
            }
            // Update power and score display
            SetCursorPosition(0, 13);
            cout << "Power: " << power << " | Score: " << score << "   ";
            // Update event message
            SetCursorPosition(0, 14);
            if (messageTimer > 0) {
                cout << "Event: " << eventMessage << "    ";
                messageTimer--;
                if (messageTimer <= 0) eventMessage = "";
            }
            else {
                cout << "Event:               ";
            }
        }

    public:
        Game() : gameRunning(false), playerX(MAP_HEIGHT - 1), playerY(MAP_WIDTH / 2), power(INITIAL_POWER), score(0), messageTimer(0) {
            for (int i = 0; i < MAP_HEIGHT; i++) {
                for (int j = 0; j < MAP_WIDTH; j++) {
                    gameMap[i][j] = ' ';
                }
            }
        }

        void Initialize() {
            lock_guard<mutex> lock(mtx);
            for (int i = 0; i < MAP_HEIGHT; i++) {
                for (int j = 0; j < MAP_WIDTH; j++) {
                    gameMap[i][j] = ' ';
                }
            }
            playerX = MAP_HEIGHT - 1;
            playerY = MAP_WIDTH / 2;
            gameMap[playerX][playerY] = 'P';
            power = INITIAL_POWER;
            score = 0;
            gameRunning = true;
            eventMessage = "";
            messageTimer = 0;
        }

        void MainMenu() {
            while (true) {
                system("cls");
                cout << "===== My Game =====\n";
                cout << "S. Start Game\n";
                cout << "L. View Leaderboard\n";
                cout << "Q. Quit\n";
                cout << "Enter your choice: ";

                char choice;
                cin >> choice;

                if (choice == 'Q' || choice == 'q') {
                    exit(0);
                }
                else if (choice == 'L' || choice == 'l') {
                    ShowLeaderboard();
                }
                else if (choice == 'S' || choice == 's') {
                    return;
                }
            }
        }

        void MovePlayer() {
            while (gameRunning) {
                if (_kbhit()) {
                    char ch = _getch();
                    {
                        lock_guard<mutex> lock(mtx);
                        ch = toupper(ch);
                        gameMap[playerX][playerY] = ' ';
                        if (ch == 'A' && playerY > 0) playerY--;
                        if (ch == 'D' && playerY < MAP_WIDTH - 1) playerY++;
                        if (ch == 'Q') gameRunning = false;
                        gameMap[playerX][playerY] = 'P';
                    }
                }
                Sleep(50);
            }
        }

        void SpawnObjects() {
            int spawnInterval = INITIAL_SPAWN_INTERVAL;
            int enemySpawnChance = INITIAL_ENEMY_SPAWN_CHANCE;

            while (gameRunning) {
                Sleep(spawnInterval);
                lock_guard<mutex> lock(mtx);

                int y = rand() % MAP_WIDTH;
                int spawnRoll = rand() % 100;

                if (gameMap[0][y] == ' ') {
                    if (spawnRoll < enemySpawnChance) {
                        gameMap[0][y] = 'X';  // Enemies
                    }
                    else if (spawnRoll < enemySpawnChance + 15) {
                        gameMap[0][y] = '*';  // Healing
                    }
                    else {
                        gameMap[0][y] = '+';  // Score
                    }
                }

                if (enemySpawnChance < MAX_ENEMY_SPAWN_CHANCE) {
                    enemySpawnChance += 2;
                }

                if (spawnInterval > MIN_SPAWN_INTERVAL) {
                    spawnInterval -= 50;
                }
            }
        }

        void MoveObjects() {
            while (gameRunning) {
                Sleep(500);
                {
                    lock_guard<mutex> lock(mtx);
                    for (int i = MAP_HEIGHT - 2; i >= 0; i--) {
                        for (int j = 0; j < MAP_WIDTH; j++) {
                            if (gameMap[i][j] == 'X' || gameMap[i][j] == '*' || gameMap[i][j] == '+') {
                                if (i + 1 < MAP_HEIGHT) {
                                    if (gameMap[i + 1][j] == 'P') {
                                        if (gameMap[i][j] == 'X') {
                                            power -= 10;
                                            score -= 5;
                                            eventMessage = "Hit by Enemy! -10 power, -5 score";
                                            messageTimer = 10;
                                            Beep(300, 300);
                                            if (power <= 0) gameRunning = false;
                                        }
                                        else if (gameMap[i][j] == '*') {
                                            power += 5;
                                            eventMessage = "Healed! +5 power";
                                            messageTimer = 10;
                                            Beep(800, 200);
                                        }
                                        else {
                                            score += 5;
                                            eventMessage = "Score Boost! +5";
                                            messageTimer = 10;
                                            Beep(600, 200);
                                        }
                                    }
                                    else if (i + 1 == MAP_HEIGHT - 1) {
                                        if (gameMap[i][j] == 'X') {
                                            eventMessage = "Enemy Escaped!";
                                            messageTimer = 10;
                                            Beep(400, 100);
                                            Beep(300, 100);
                                        }
                                        gameMap[i][j] = ' ';
                                        continue;
                                    }
                                    else {
                                        gameMap[i + 1][j] = gameMap[i][j];
                                    }
                                }
                                gameMap[i][j] = ' ';
                            }
                        }
                    }
                }
            }
        }

        void PowerDecay() {
            int decayRate = INITIAL_DECAY_RATE;
            while (gameRunning) {
                Sleep(DECAY_INTERVAL);
                lock_guard<mutex> lock(mtx);
                power -= decayRate;
                score += 2;
                if (power <= 0) gameRunning = false;
                if (decayRate < MAX_DECAY_RATE) {
                    decayRate++;
                }
            }
        }

        void GameLoop() {
            DrawStaticBackground();
            while (gameRunning) {
                Sleep(FRAME_RATE);
                DrawDynamicElements();
            }
        }

        // Accessors for GameThread.cpp
        int GetPower() const { return power; }
        int GetScore() const { return score; }
        void SaveLeaderboard(const string& name, int score) {
            ofstream file("leaderboard.txt", ios::app);
            file << name << " " << score << endl;
            file.close();
        }
    };

    // Global Game instance
    Game game;

    // Wrapper functions to call Game methods
    void MainMenu() { game.MainMenu(); }
    void InitializeGame() { game.Initialize(); }
    void MovePlayer() { game.MovePlayer(); }
    void SpawnObjects() { game.SpawnObjects(); }
    void MoveObjects() { game.MoveObjects(); }
    void PowerDecay() { game.PowerDecay(); }
    void GameLoop() { game.GameLoop(); }
    int GetPower() { return game.GetPower(); }
    int GetScore() { return game.GetScore(); }
    void SaveLeaderboard(const string& name, int score) { game.SaveLeaderboard(name, score); }
}