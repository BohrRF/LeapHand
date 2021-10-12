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

int64_t control::calBeatLen()
{
    return 1000000 * (60 / curBpm);
}

void control::resetPlayState()
{
    beat_ptr = music.beatSet.begin();
    node_ptr = beat_ptr->tickSet.begin();
    playState = false;
    onPlayList.clear();
}

const double STDRANGE = 300.0;
const double STDACCEL = 100000;

void control::onBeat(const int64_t& curTimeStamp, const double & bpm ,const double& hand_amp, const double& hand_accel)
{
    curVelocityFactor = hand_amp / STDRANGE;//TODO change it into factor form
    curAccel = hand_accel;
    cout << "curAccel " << curAccel << endl;
    curSpanFactor = curAccel / STDACCEL;//TODO scale this
    curBpm = (abs(bpm - curBpm) > 5) ? bpm : curBpm;
    
    if (!autoplayMode) 
    {
        // there is some same operation in freash but this meant to excecute no matter node_ptr reached the end of this beat or not
        if (++beat_ptr == music.beatSet.end())
        {
            beat_ptr = music.beatSet.begin();
            if (!musicLoop) playState = false;
        }
        node_ptr = beat_ptr->tickSet.begin();
       
        // in case the first note isn't at the first place of this beat
        curNodeTimeStamp = curTimeStamp + node_ptr->tickOffset * 1000000 * (60 / curBpm) / beat_ptr->tickBeatLength;
    }

    refresh(curTimeStamp);
}

void control::refresh(const int64_t& curTimeStamp)
{
    //cout << curTimeStamp << " " << curBpm << endl;
    if (!playState) return;

    bool isListChange = false;

    if (curTimeStamp >= curNodeTimeStamp)
    {
        int64_t nsPerTick = 1000000 * (60 / curBpm) / beat_ptr->tickBeatLength;

        for (auto& n : node_ptr->notes)
        {
            //play
            //cout << "c " << n.channel << ' ' << "n " << n.number << ' ' << "t " << n.tickSpan << ' ' << "v " << n.velocity << '\n' << endl;
            cout << n.velocity * curVelocityFactor << endl;

            playSound.noteOn(n.number, n.velocity * curVelocityFactor);
            onPlayNote noteTemp;
            noteTemp.onNote = n;
            noteTemp.endTimeStamp = curTimeStamp + n.tickSpan * nsPerTick * curSpanFactor;

            cout <<"time stamp" << curTimeStamp << " span" << n.tickSpan * nsPerTick * curSpanFactor << endl;

            onPlayList.emplace_back(noteTemp);
            isListChange = true;
        }

        unsigned int offSetTemp = node_ptr->tickOffset;
        unsigned int beatLengthTemp = beat_ptr->tickBeatLength;
        
        if (++node_ptr == beat_ptr->tickSet.end()) // the end of this beat?
        {
            if (autoplayMode)
            {
                if (++beat_ptr == music.beatSet.end()) // end of the whole music?
                {
                    beat_ptr = music.beatSet.begin();
                    if (!musicLoop) playState = false;
                }
                node_ptr = beat_ptr->tickSet.begin();

                                     // first node offset in new beat + previous beat length - last beat offtset in previous beat
                curNodeTimeStamp = curTimeStamp + (node_ptr->tickOffset + beatLengthTemp - offSetTemp) * nsPerTick;
            }
        }
        else
        {
            // note that node_ptr has already been moved to the next during calculating the condition
            curNodeTimeStamp = curTimeStamp + (node_ptr->tickOffset - offSetTemp) * nsPerTick;
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
}

