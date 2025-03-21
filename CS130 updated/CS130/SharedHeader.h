#pragma once
#include <mutex>
#include <string>
namespace mygame
{
    void MovePlayer();      // Thread to handle player movement
    void SpawnObjects();    // Thread to spawn enemies and items
    void MoveObjects();     // Thread to move objects downward
    void GameLoop();        // Thread to update and draw the game
    void PowerDecay();      // Thread to reduce power over time
    void InitializeGame();  // Initialize the game state
    void DrawBackground(); // Draw static parts once

    extern int power;
    extern int score;       // Player's score
    extern char gameMap[10][20]; // Game grid (10 rows, 20 columns)
    extern int playerX, playerY; // Player position
    extern std::mutex mtx;       // Mutex for thread synchronization
    extern bool gameRunning;     // Flag to control game loop
    extern std::string eventMessage; // Temporary event message
    extern int messageTimer;    // Timer for displaying event message
}