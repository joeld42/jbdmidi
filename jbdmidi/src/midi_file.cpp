//
//  midi_file.cpp
//  jbdmidi
//
//  Created by Joel Davis on 3/13/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <iostream>
#include <stdint.h>

#include "midi_event.h"
#include "midi_file.h"

using namespace jbdmidi;

// Midi "Meta" Events
enum 
{
    MetaEvent_SEQNUM     = 0x00, // Sequence Number (or Pattern Number)
    MetaEvent_TEXT       = 0x01, // Track Text
    MetaEvent_COPYRIGHT  = 0x02, // Copyright Text
    MetaEvent_SEQNAME    = 0x03, // Sequence/Track Name
    MetaEvent_INSTNAME   = 0x04, // Instrument Name
    MetaEvent_LYRICS     = 0x05, // Lyrics Text
    MetaEvent_MARKER     = 0x06, // Marker (often verse/chorus)
    MetaEvent_CUE        = 0x07, // Cue Point
    
    MetaEvent_CHANPREFIX = 0x20, // Channel Prefix (eg. before instrument name)
    
    MetaEvent_ENDOFTRACK = 0x2F, // End of track marker
    
    MetaEvent_SETTEMPO   = 0x51, // Set Tempo

    MetaEvent_SMPTE      = 0x54, // SMPTE Offset (not supported)
    
    MetaEvent_TIMESIG    = 0x58, // Set Time signature
    MetaEvent_KEYSIG     = 0x59, // Key Signature
    
    MetaEvent_SEQDATA    = 0x7F  // Sequencer specific data
};

// TODO: ifdef this for other platforms

inline void endian_swap(uint16_t &x)
{
    x = (x>>8) | 
    (x<<8);
}

inline void endian_swap(uint32_t &x)
{
    x = (x>>24) | 
    ((x<<8) & 0x00FF0000) |
    ((x>>8) & 0x0000FF00) |
    (x<<24);
}

inline void endian_swap(int64_t &x)
{
    x = (x>>56) | 
    ((x<<40) & 0x00FF000000000000) |
    ((x<<24) & 0x0000FF0000000000) |
    ((x<<8)  & 0x000000FF00000000) |
    ((x>>8)  & 0x00000000FF000000) |
    ((x>>24) & 0x0000000000FF0000) |
    ((x>>40) & 0x000000000000FF00) |
    (x<<56);
}

// =========================================================================
// Structure of MIDI file
// =========================================================================

// don't align these structs since they're used for file interpretation
#pragma pack (push,1)

struct MIDI_HEADER
{
    char headerChunkId[4];  // Header chunk, should be "MThd"
    uint32_t m_chunkSize;     // chunk size, should be 0x0000000006
    uint16_t m_formatType;    // type 0 (one track), 1 or 2 MIDI
    uint16_t m_numTracks;     // number of tracks
    uint16_t m_timeDivision;  // Time divisor info (special encoding)   
};

static void endian_swap( MIDI_HEADER &hdr )
{
    endian_swap( hdr.m_chunkSize );
    endian_swap( hdr.m_formatType );
    endian_swap( hdr.m_numTracks );
    endian_swap( hdr.m_timeDivision );
}

// -------------------------------------------------------------------------
struct MIDI_TRACK_HEADER
{
    char trackChunkId[4]; // Track chunk, should be "MTrk"
    uint32_t m_chunkSize; // chunkSize size of track with event data    
};

static void endian_swap( MIDI_TRACK_HEADER &hdr )
{
    endian_swap( hdr.m_chunkSize );
}

// -------------------------------------------------------------------------

// Midi event data after the time value
struct MIDI_EVENT
{
    uint8_t m_status; // sysex, meta or event,chan
    uint8_t m_param1; // param1, type dependant
    uint8_t m_param2; // param2
};


// done with file structs
#pragma pack (pop)

// =========================================================================

// Unpacks a midi "variable length" value
static uint8_t *unpackVarLen( uint8_t *p, uint32_t &result )
{
    result = *p & 0x7f;
    
    // shift left and keep going if the high bit is set
    while (*p & 0x80)
    {
        ++p;
        result = (result << 7) | (*p & 0x7f);
    }
        
    return ++p;
}

MidiFile *jbdmidi::readMidiFile( const char *filename )
{
    MidiFile *midifile = NULL;

    FILE *fp = fopen( filename, "rb" );
    
    if (!fp)
    {
        printf("Error: couldn't open file '%s'\n", filename );
        return NULL;
    }
    
    midifile = new MidiFile();
    
    // Read the file header
    MIDI_HEADER header;
    fread( &header, sizeof( MIDI_HEADER ), 1, fp );
    endian_swap( header );
    
    printf("header size %lu\n", sizeof( MIDI_HEADER ) );
    printf("Header chunk: %c%c%c%c\n", 
           header.headerChunkId[0],
           header.headerChunkId[1],
           header.headerChunkId[2],
           header.headerChunkId[3] );
    
    printf("chunkSize (should be 6) %d\n", header.m_chunkSize );
    printf("formatType : %d\n", header.m_formatType );
    printf("numTracks : %d\n", header.m_numTracks );
    printf("timeDivision : 0x%04X\n", header.m_timeDivision );    

    // Read each midi track chunk
    MIDI_TRACK_HEADER trackHeader;
    for (uint16_t i=0; i < header.m_numTracks; i++)
    {
        fread( &trackHeader, sizeof(MIDI_TRACK_HEADER), 1, fp );
        endian_swap( trackHeader );
        
        printf("Track %d --- ", i );
        
        printf("Track chunk: %c%c%c%c\n", 
               trackHeader.trackChunkId[0],
               trackHeader.trackChunkId[1],
               trackHeader.trackChunkId[2],
               trackHeader.trackChunkId[3] );
        printf("chunkSize  %d\n", trackHeader.m_chunkSize );
        
        // Read the event data
        void *eventData = malloc( trackHeader.m_chunkSize );
        fread( eventData, trackHeader.m_chunkSize, 1, fp );
        
        // interpret the event data
        uint8_t *curr = (uint8_t*)eventData;

        while (curr < (uint8_t*)(eventData)+trackHeader.m_chunkSize)
        {
            
//            MetaEvent_SEQNUM     = 0x00, // Sequence Number (or Pattern Number)
//            MetaEvent_TEXT       = 0x01, // Track Text
//            MetaEvent_COPYRIGHT  = 0x02, // Copyright Text
//            MetaEvent_SEQNAME    = 0x03, // Sequence/Track Name
//            MetaEvent_INSTNAME   = 0x04, // Instrument Name
//            MetaEvent_LYRICS     = 0x05, // Lyrics Text
//            MetaEvent_MARKER     = 0x06, // Marker (often verse/chorus)
//            MetaEvent_CUE        = 0x07, // Cue Point
//            
//            MetaEvent_CHANPREFIX = 0x20, // Channel Prefix (eg. before instrument name)
//            
//            MetaEvent_ENDOFTRACK = 0x2F, // End of track marker
//            
//            MetaEvent_SETTEMPO   = 0x51, // Set Tempo
//            
//            MetaEvent_SMPTE      = 0x54, // SMPTE Offset (not supported)
//            
//            MetaEvent_TIMESIG    = 0x58, // Set Time signature
//            MetaEvent_KEYSIG     = 0x59, // Key Signature
//            
//            MetaEvent_SEQDATA    = 0x7F  // Sequencer 
            
            // Get event time value
            uint32_t timeVal;
            curr = unpackVarLen( curr, timeVal );                    
            
            // Get event
            MIDI_EVENT *event = (MIDI_EVENT*)curr;
            printf("Status byte: 0x%02X\n", event->m_status );
            if (event->m_status == 0xFF)
            {
                printf( "meta: 0x%02X\n", event->m_param1 );
                // Skip Meta + event
                curr += 2;
                
                // "meta" event
                switch (event->m_param1)
                {
                    case MetaEvent_SEQNUM:
                        {
                            uint16_t seqNum = *(uint16_t*)curr;
                            endian_swap(seqNum);
                            printf( "  t: 0x%04X Sequence Number: %d", timeVal, seqNum );
                            curr += 4;
                        }
                        break;
                        
                    case MetaEvent_TEXT:
                        break;
                        
                    case MetaEvent_TIMESIG:
                        {                            
                            uint8_t numerator     = *curr++;
                            uint8_t denominator   = *curr++;
                            uint8_t metronome     = *curr++;
                            uint8_t thirtySeconds = *curr++;
                            printf( "  t: 0x%04X Time Signature: %d/%d metronome %d 32nds %d\n", 
                                   timeVal, numerator, denominator, metronome, thirtySeconds );
                            curr += 4;
                        }
                        break;
                        
                }
            }
            else if (event->m_status==0xF0)
            {
                // SYSEX event
            }
            else 
            {
                // Regular event            
                int eventType = (event->m_status & 0xF0) >> 4;
                int eventChannel = (event->m_status & 0x0F);
            
                printf( "  t: 0x%04X EV: 0x%X channel %d - %d %d\n",
                       timeVal, eventType, eventChannel, event->m_param1, event->m_param2 );
            
                // Move to next event
                curr += sizeof( MIDI_EVENT );
            }
        }

        
        
        // done with this track chunk
        free( eventData );
    }
    
//    while (!feof(fp))
//    {
//        MidiEvent *currEvent = NULL;        
//                
//    }
    
    return midifile;
}

