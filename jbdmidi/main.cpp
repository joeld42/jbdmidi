//
//  main.cpp
//  jbdmidi
//
//  Created by Joel Davis on 3/13/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <iostream>

#include "midi_file.h"
#include "midi_event.h"

using namespace jbdmidi;

int main (int argc, const char * argv[])
{
    
    MidiFile *testmidi;
    testmidi = readMidiFile( "testdata/ff6aria.mid" );
    if (!testmidi)
    {
        printf("Could not read midi file." );
        return 1;
    }
    
    for (MidiEvent *currEvent = testmidi->m_events; 
         currEvent; currEvent = currEvent->m_next )
    {
        
    }
    
    return 0;
}

