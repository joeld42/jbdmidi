//
//  midi_event.cpp
//  jbdmidi
//
//  Created by Joel Davis on 3/13/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <iostream>

#include "midi_event.h"

using namespace jbdmidi;

MidiEvent::MidiEvent( uint8_t eventType ) :
    m_eventType( eventType ),
    m_next( NULL )
{
    
}