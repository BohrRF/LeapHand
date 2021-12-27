#ifndef PLAYCONTROL_H_INCLUDED
#define PLAYCONTROL_H_INCLUDED

#include <string>
#include <list>
#include <iostream>
#include "dataBase.h"
#include "midi.h"
#include "Fourier.h"
#include "bpmLinkedList.h"


const int BPM_LEN = 12;
const double LOW_HAND_AMP = 10;

class onPlayNote
{
public:
    note onNote;
    int64_t endTimeStamp = 0;
    onPlayNote() {}
    onPlayNote(const note &n, const int64_t & ts) : onNote(n), endTimeStamp(ts) {}
};

class control
{
    unsigned cnt = 0;
    std::vector<double> cali_ary;

    midiOut midi;
    std::list<onPlayNote> onPlayList;

    musicData music;
    std::vector<tickNode>::iterator node_ptr;
    std::vector<beatSection>::iterator beat_ptr;
    std::list<onPlayNote>::iterator onPlay_ptr = onPlayList.begin();

    

    int64_t curNodeTimeStamp = 0;
    int64_t lastBeatTimeStamp = 0;
    int64_t nextBeatTimeStamp = 0;
    int64_t pauseTimeStamp = 0;

    unsigned beat_count = 0;

    void setplayState(const int64_t& curTimeStamp, const int& mode);
    void resetBeatPos();

    
    unsigned int timetrans(const unsigned int& len);
    void write_beat(beatSection &beat, const std::vector<int> &notes);
    

public:
    BpmList bpmList;
    bool autoplayMode = true;
    bool musicLoop = true;
    bool speedFix = false;
    bool isCalib = false;
    unsigned int gearFactor = 1;

    double timeOffset = 142.404;
    double HAND_MAX = 235.569;
    double HAND_MIN = 29.0981;
    double ACCEL_STD = 0.896602;
    double ACCEL_MAX = 1.11887;
    double ACCEL_MIN = 0.65;

    int curBeatPos = 1;
    unsigned char playState = 0;
    bool beatRecieved = false;
    double handBpm = 0;
    double curAmp = 0;
    double curAccel = 1;
    double curBpm = 120;
    double curSpanFactor = 1;
    double curVelocityFactor = 1;
    double dif = 0;
    double speedBias = 0;

    int readMIDI(std::string FILENAME);

    void printMusic() const;
    void printParameter() const;
    void getOnPlayList(unsigned char ary[]) const;

    void start(const int64_t &curTimeStamp);
    void pause(const int64_t &curTimeStamp);
    void resume(const int64_t &curTimeStamp);
    void restart(const int64_t &curTimeStamp);
    void hover(const int64_t& curTimeStamp);
    
    void resetAll();

    void onBeat(const int64_t& TimeStamp, const double& hand_amp, const double& hand_accel, const Fourier& fft);
    int refresh(const int64_t& TimeStamp, const Fourier& fft);

    void initial_music();
    void calibrate(const int64_t& TimeStamp, const double& hand_amp, const double& hand_accel, const Fourier& fft);
    const unsigned& calibrate_next();

    control() : bpmList(BPM_LEN){}
};

#endif // PLAYCONTROL_H_INCLUDED
