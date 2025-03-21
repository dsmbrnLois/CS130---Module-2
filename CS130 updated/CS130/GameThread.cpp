#include <iostream>
#include <thread>
#include <windows.h>
#include "SharedHeader.h"

using namespace std;

int main()
{
    mygame::InitializeGame();

    // Start threads
    thread t1(mygame::GameLoop);
    thread t2(mygame::MovePlayer);
    thread t3(mygame::SpawnObjects);
    thread t4(mygame::MoveObjects);
    thread t5(mygame::PowerDecay);

    // Wait for threads to finish
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    // Play game over sound
    Beep(500, 200); // Game over sequence
    Beep(400, 200);
    Beep(300, 300);

    cout << "Game Over! Final Power: " << mygame::power << " | Final Score: " << mygame::score << endl;
    return 0;
}