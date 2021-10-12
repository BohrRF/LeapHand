#include"midi.h"

void simpleSound::noteOn(unsigned iNote, unsigned iVelocity)
{
    unsigned packdata = 0x90 | (iNote << 8) | (iVelocity << 16);
    midiOutShortMsg(ghMidiOut, packdata);
}

void simpleSound::noteOff(unsigned iNote)
{
    unsigned packdata = 0x80 | (iNote << 8);
    midiOutShortMsg(ghMidiOut, packdata);
}

simpleSound::simpleSound()
{
    MMRESULT mmres = midiOutOpen(&ghMidiOut, MIDI_MAPPER, NULL, NULL, CALLBACK_NULL);
    if (mmres != MMSYSERR_NOERROR)
        fprintf(stderr, "MIDI‚ª—˜—p‚Å‚«‚Ü‚¹‚ñB\n");
}

simpleSound::~simpleSound()
{
    midiOutClose(ghMidiOut);
}
