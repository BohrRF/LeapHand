#include "playControl.h"
#include <iostream>

using std::cout;
using std::endl;

unsigned int control::timetrans(const unsigned int &den)
{
    return TPQN * 4 / den;
}

void control::write_beat(beatSection &beat, const std::vector<int> &notes)
{
    note note_temp;
    tickNode notes_temp;
    unsigned int tick_count = 0;

    beat.setTimeSigniture(4, 4);
    for(auto it = notes.begin(); it != notes.end(); it++)
    {
        note_temp.channel = 0;
        note_temp.number = *it++;
        note_temp.tickSpan = timetrans(*it);
        note_temp.velocity = 72;

        notes_temp.notes.emplace_back(note_temp);
        notes_temp.tickOffset = tick_count;
        tick_count += note_temp.tickSpan;
        beat.tickSet.emplace_back(notes_temp);

        notes_temp.notes.clear();
    }
}

void control::initial_music()
{
    beatSection beat_temp;
    write_beat(beat_temp, {60, 16, 64, 16, 67, 16, 72, 16});
    for(int i = 0; i < 16; i++)
        music.beatSet.emplace_back(beat_temp);
    beat_ptr = music.beatSet.begin();
    node_ptr = (*beat_ptr).tickSet.begin();
    playState = false;
}

void control::printMusic()
{
    int bar_num = 1;
    for(const auto &bar : music.beatSet)
    {
        cout << "Beat: " << bar_num++ << endl;
        for(const auto &tick : bar.tickSet)
        {
            cout << "\tTick Offset: "<< tick.tickOffset << endl;
            for(const auto &note : tick.notes)
            {
                cout << "\t\t"
                     << "c " << note.channel << ' '
                     << "n " << note.number << ' '
                     << "t " << note.tickSpan << ' '
                     << "v " << note.velocity << '\n' << endl;
            }
        }
    }
}

std::pair<int, int> control::getMusicInfo()
{
    std::pair<int, int> timesig = std::make_pair(beat_ptr->timeSigniture_num, beat_ptr->timeSigniture_den);
    return timesig;
}

int64_t control::calBeatLen()
{
    return 1000000 * (60 / curBpm);
}

void control::setplayState(const int64_t &curTimeStamp, const int& mode)
{
    if (mode == 2)
    {
        lastBeatTimeStamp = curNodeTimeStamp = curTimeStamp;
    }
    playState = mode;
}

void control::resetPlayState()
{
    beat_ptr = music.beatSet.begin();
    node_ptr = beat_ptr->tickSet.begin();
    playState = false;
    onPlayList.clear();
}

void control::resetBeat()
{
    node_ptr = beat_ptr->tickSet.begin();
    playState = false;
    onPlayList.clear();
}

void control::resetBpmList()
{
    bpmList.clear();
}

const double STDRANGE = 300.0;
const double STDACCEL = 10;

void control::onBeat(const int64_t& curTimeStamp, const double& hand_amp, const double& hand_accel ,const Fourier& fft)
{
    if (hand_amp < 10) return;

    curVelocityFactor = hand_amp/STDRANGE;//TODO change it into factor form

    curAccel = hand_accel;

    curSpanFactor = STDACCEL / curAccel;//TODO scale this
system("cls");
    bpmList.push(curTimeStamp);

    fft.FFT();
    std::vector<std::pair<double, double>> maxlist;
    
    fft.find_max_freq(maxlist); // 0 Hz amplitude was eliminated from this list
    /*
    system("cls");
    cout << "curAccel " << curAccel << endl;
    cout << "curhandamp" << hand_amp << endl;
    cout << "curBPM" << bpm << endl;
    
    //cout << "cur time: " << curTimeStamp << " period: " << 8LL * 1000000 / maxlist[0] << '\n';
    
    for (int i = 0; i < 5; i++)
    {
        printf("%f, %f\n", 60 * maxlist[i].first, maxlist[i].second);
    }
    //curBpm = 60 * maxlist[0].first;
    */
    curBpm = bpmList.calAverage(beat_ptr->timeSigniture_num);
    cout << curBpm << endl;

    if (playState == 3)
    {
        setplayState(curTimeStamp, 2);
    }
    
    if (!autoplayMode) 
    {
        // there is some same operation in freash but this meant to excecute no matter node_ptr reached the end of this beat or not
        if (beat_ptr != music.beatSet.begin() && ++beat_ptr == music.beatSet.end())
        {
            beat_ptr = music.beatSet.begin();
            if (!musicLoop) playState = 0;
        }
        node_ptr = beat_ptr->tickSet.begin();
       
        // in case the first note isn't at the first place of this beat
        curNodeTimeStamp += node_ptr->tickOffset * 1e6 * (60 / curBpm) / beat_ptr->tickBeatLength;
    }
    else
    {
        double dif = 1.0 * (curTimeStamp - lastBeatTimeStamp) / (1e6 * (60.0 / curBpm));
        cout << dif << endl;
        if (dif > 0.25 && dif < 0.75)
        {
            playState = 3;
        }
    }
}

int control::refresh(const int64_t& curTimeStamp, const Fourier& fft)
{
    
    //cout << curTimeStamp << " " << curBpm << endl;
    
    /*isBeatEnter => waiting for next Beat, set to true every onbeat()
    * curTimeStamp > lastBeatTimeStamp + 1e6 * (60.0 / curBpm) => if next beat should already came
    * hand_amp < 40 => hand_amp is too small to recongnized as a beat is comming
    */

    /* 
        0 -> not change 
        1 -> next node
        2 -> next beat
    */
    int if_next = 0; 
    
    ///TODO: stop the replay, go to (pause processing sequence) OR (a tempo and start right after next hand beat)


    if (playState != 2) return if_next;
    bool isListChange = false;
    if (curTimeStamp >= curNodeTimeStamp)
    {
        if_next = 1;
        int64_t usPerTick = 1e6 * (60.0 / curBpm) / beat_ptr->tickBeatLength;
        if (node_ptr != beat_ptr->tickSet.end())
        {
            if_next = 2;
            for (auto& n : node_ptr->notes)
            {
                //play
                //cout << "c " << n.channel << ' ' << "n " << n.number << ' ' << "t " << n.tickSpan << ' ' << "v " << n.velocity << '\n' << endl;
                //cout << n.velocity * curVelocityFactor << endl;

                playSound.noteOn(n.number, n.velocity * curVelocityFactor);
                onPlayNote noteTemp;
                noteTemp.onNote = n;
                noteTemp.endTimeStamp = curNodeTimeStamp + n.tickSpan * usPerTick * curSpanFactor;
                //cout << "time stamp" << curTimeStamp << " span" << n.tickSpan * nsPerTick * curSpanFactor << endl;

                onPlayList.emplace_back(noteTemp);
                isListChange = true;
            }

            unsigned int offSetTemp = node_ptr->tickOffset;
            unsigned int beatLengthTemp = beat_ptr->tickBeatLength;
            //cout << curTimeStamp << endl;
            //cout << curNodeTimeStamp << "->";
            if (++node_ptr == beat_ptr->tickSet.end()) // the end of this beat?
            {
                if (autoplayMode)
                {
                    if (++beat_ptr == music.beatSet.end()) // end of the whole music?
                    {
                        beat_ptr = music.beatSet.begin();
                        if (!musicLoop) playState = 0;
                    }
                    node_ptr = beat_ptr->tickSet.begin();

                    // first node offset in new beat + previous beat length - last beat offtset in previous beat
                    curNodeTimeStamp += (node_ptr->tickOffset + beatLengthTemp - offSetTemp) * usPerTick;
                    lastBeatTimeStamp = curNodeTimeStamp - node_ptr->tickOffset;
                    if (fft.calHightRatio() > 0.7)
                    {
                        //playState = 3;
                    }
                }
            }
            else
            {   
                // node_ptr has already been moved to the next during the calculation of the condition
                curNodeTimeStamp += (node_ptr->tickOffset - offSetTemp) * usPerTick;
            }
            //cout << curNodeTimeStamp << endl;
        }
    }

    std::list<onPlayNote>::iterator it = onPlayList.begin();
    while (it != onPlayList.end())
    {
        if (it->endTimeStamp <= curTimeStamp)
        {
            //stop music
            playSound.noteOff(it->onNote.number);
            it = onPlayList.erase(it);

            isListChange = true;
        }
        else
            it++;
    }
    /*
    if (isListChange)
    {
        system("cls");
        cout << "Current Bpm " << curBpm << endl;
        for (const auto& a : onPlayList)
        {    
            cout << "c " << a.onNote.channel << ' ' << "n " << a.onNote.number << ' ' << "t " << a.onNote.tickSpan << ' ' << "v " << a.onNote.velocity << " end " << a.endTimeStamp << '\n' << endl;
        }
    }
    */
    return if_next;
}

