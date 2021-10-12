#include <iostream>
#include "playControl.h"
#include <chrono>
#include <windows.h>
using namespace std;

int main()
{
    control con;
    con.initial_music();
    con.playState = true;
    bool ifKeepE = false;
    while(1)
    {
        auto d = std::chrono::high_resolution_clock::now().time_since_epoch();
        con.refresh(d.count(), 15);
        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            if(!ifKeepE)
            {
                con.onBeat(d.count(), 15);
                ifKeepE = true;
            }
        }
        else
        {
            ifKeepE = false;
        }
    }
}
