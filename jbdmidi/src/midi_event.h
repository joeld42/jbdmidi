#ifndef jbdmidi_midi_event_h
#define jbdmidi_midi_event_h

//
//  midi_event.h
//  jbdmidi
//
//  Created by Joel Davis on 3/13/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//


#include <stdint.h>

namespace jbdmidi
{

enum
{
    MidiEvent_NOTEON,
    MidiEvent_NOTEOFF
};
    
struct MidiEvent
{
public:  
    MidiEvent( uint8_t eventType );
    
    uint8_t m_eventType;
    MidiEvent *m_next;   
};
    
}
#endif
