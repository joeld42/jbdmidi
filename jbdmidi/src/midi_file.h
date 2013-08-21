#ifndef jbdmidi_midi_file_h
#define jbdmidi_midi_file_h

//
//  midi_file.h
//  jbdmidi
//
//  Created by Joel Davis on 3/13/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//


namespace jbdmidi
{

struct MidiEvent;

// "header" information 
struct MidiFile
{
    size_t m_numTracks;
    MidiEvent *m_events;
};
    
// Returns a list of midi events, or NULL if the 
// file can't be read
MidiFile *readMidiFile( const char *filename );

};

#endif
