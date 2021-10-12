#pragma once
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

class simpleSound
{
    HMIDIOUT ghMidiOut;
public:
    void noteOn(unsigned iNote, unsigned iVelocity);
    void noteOff(unsigned iNote);
    simpleSound();
    ~simpleSound();
};
