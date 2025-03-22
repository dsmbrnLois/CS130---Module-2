#pragma once
#include <string>

namespace mygame {
    class Game;

    // Functions that need to be called from GameThread.cpp
    void MainMenu();
    void InitializeGame();
    void MovePlayer();
    void SpawnObjects();
    void MoveObjects();
    void PowerDecay();
    void GameLoop();

    // Functions to access game state
    int GetPower();
    int GetScore();
    void SaveLeaderboard(const std::string& name, int score);
}