#ifndef PLAYCONTROL_H_INCLUDED
#define PLAYCONTROL_H_INCLUDED

#include <string>
#include <list>
#include "dataBase.h"
#include "midi.h"
#include "Fourier.h"

class onPlayNote
{
public:
    note onNote;
    int64_t endTimeStamp;
    onPlayNote(){}
};

class control
{
    musicData music;



    std::list<onPlayNote> onPlayList;

    simpleSound playSound;

    int64_t curNodeTimeStamp; //update when play node_ptr jump
    std::vector<tickNode>::iterator node_ptr;

    unsigned int tickCount; //reset when beat_ptr jump
    std::vector<beatSection>::iterator beat_ptr;

    unsigned int timetrans(const unsigned int& len);
    void write_beat(beatSection &beat, const std::vector<int> &notes);
    double curBpm;
    double curVelocityFactor;
    double curSpanFactor;
    double curAccel;
    int64_t nextBeatTimeStamp;
    bool autoplayMode;
    bool musicLoop;
    bool isBeatEnter;
public:
    unsigned char beat_count = 0;
    int64_t calBeatLen();
    control() :
        curBpm(0.0),
        curVelocityFactor(0.0),
        curSpanFactor(0.0),
        curAccel(0.0),
        nextBeatTimeStamp(9635826323000000),
        autoplayMode(true), 
        musicLoop(true), 
        isBeatEnter(false)
        {}
    unsigned char playState;
    void readtxt(std::string FILENAME);
    void printMusic();
    void initial_music();
    std::pair<int, int> getMusicInfo();
    void resetPlayState();
    void resetBeat();

    void onBeat(const int64_t& curTimeStamp, const double& hand_amp, const double& hand_accel, const Fourier& fft);
    void refresh(const int64_t& curTimeStamp, const Fourier& fft);
};

#endif // PLAYCONTROL_H_INCLUDED
