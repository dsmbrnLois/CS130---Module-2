#include <iostream>
#include <thread>
#include <windows.h>
#include "SharedHeader.h"

using namespace std;

int main() {
    while (true) {
        // Display Main Menu
        mygame::MainMenu();

        // Initialize Game AFTER selecting "S"
        mygame::InitializeGame();

        // Start game threads
        thread t1(mygame::GameLoop);
        thread t2(mygame::MovePlayer);
        thread t3(mygame::SpawnObjects);
        thread t4(mygame::MoveObjects);
        thread t5(mygame::PowerDecay);

        // Wait for all threads to complete
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();

        // Play game over sound
        Beep(500, 200);
        Beep(400, 200);
        Beep(300, 300);

        // Game Over Screen
        cout << "Game Over! Final Power: " << mygame::GetPower() << " | Final Score: " << mygame::GetScore() << endl;

        // Ask player for their name
        cout << "Enter your name: ";
        string playerName;
        cin >> playerName;

        // Save to leaderboard
        mygame::SaveLeaderboard(playerName, mygame::GetScore());
        cout << "Score saved! Returning to menu...\n";
        Sleep(2000);
    }

    return 0;
}