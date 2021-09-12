#ifndef __EVENTS_H
#define __EVENTS_H

#include "FliM.h"
#include "module.h"
/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

events.h - Definitions for CBUS event handling module - Part of CBUS libraries for PIC 18F
 
  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                   and indicate if changes were made. You may do so in any reasonable manner,
                   but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                  your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                  legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

**************************************************************************************************************
  Note:   This source code has been written using a tab stop and indentation setting
          of 4 characters. To see everything lined up correctly, please set your
          IDE or text editor to the same settings.
******************************************************************************************************
	
 For library version number and revision history see CBUSLib.h

 Adapted to support Produced events by Ian Hogg 23/5/2017

*/

/* EVENTS
 *
 * The events are stored in tables in flash (flash is faster to read than EEPROM).
 * Separate tables are used for Produced events and Consumed events since they require 
 * different data and different lookup schemes.
 * The Produced events are looked up by the action that caused them.
 * The Consumed events are looked up by the CBUS event to find the module specific actions
 * to perform. The lookup is done using a hash table to find the index into the event2Actions table.
 * The action2Event and event2Actions tables are stored in Flash whilst the hashtable
 * lookup for the event2Actions table is stored in RAM.
 * 
 * For Produced events the event is taught using the action stored in the EV field of the
 * CBUS message.
 * For Consumed events the actions are taught using the EV field of the CBUS message.
 * Multiple actions can be specified for a Consumed event. This can be used to set up
 * routes with a single event. 
 *
 * The generic FLiM library code handles the teaching (Learn) and unlearning of events
 * and the storage of the events. The application code just needs to process a consumed 
 * event's actions and to produce actions using the application's actions.
 *
 */
// A helper structure to store the details of an event.
typedef struct {
    WORD NN;
    WORD EN;
} Event;


typedef union
{
    struct
    {
        unsigned char eVsUsed:4;  // How many of the EVs in this row are used. Only valid if continued is clear
        BOOL    continued:1;    // there is another entry 
        BOOL    continuation:1; // Continuation of previous event entry
        BOOL    forceOwnNN:1;   // Ignore the specified NN and use module's own NN
        BOOL    freeEntry:1;    // this row in the table is not used - takes priority over other flags
    };
    BYTE    asByte;       // Set to 0xFF for free entry, initially set to zero for entry in use, then producer flag set if required.
} EventTableFlags;

typedef struct {
    EventTableFlags flags;          // put first so could potentially use the Event bytes for EVs in subsequent rows.
    BYTE next;                      // index to continuation also indicates if entry is free
    Event event;                    // the NN and EN
    BYTE evs[EVENT_TABLE_WIDTH];    // EVENT_TABLE_WIDTH is maximum of 15 as we have 4 bits of maxEvUsed
} EventTable;


// EVENT DECODING
//    An event opcode has bits 4 and 7 set, bits 1 and 2 clear
//    An ON event opcode also has bit 0 clear
//    An OFF event opcode also has bit 0 set
//
//  eg:
//  ACON  90    1001 0000
//  ACOF  91    1001 0001
//  ASON  98    1001 1000
//  ASOF  99    1001 1001
//
//  ACON1 B0    1011 0000
//  ACOF1 B1    1011 0001
//  ASON1 B8    1011 1000
//  ASOF1 B9    1011 1001
//
//  ACON2 D0    1101 0000
//  ACOF2 D1    1101 0001
//  ASON2 D8    1101 1000
//  ASOF2 D9    1101 1001
//
//  ACON3 F0    1111 0000
//  ACOF3 F1    1111 0001
//  ASON3 F8    1111 1000
//  ASOF3 F9    1111 1001
//
// ON/OFF         determined by d0
// Long/Short     determined by d3
// num data bytes determined by d7,d6
//
#define     EVENT_SET_MASK   0b10010000
#define     EVENT_CLR_MASK   0b00000110
#define     EVENT_ON_MASK    0b00000001
#define     EVENT_SHORT_MASK 0b00001000

#define EV_FILL             0
#define NO_INDEX            0xff

// Function prototypes for event management

//void 	eventsInit( void );
extern void clearAllEvents(void);
extern void clearAction2Event(void);
extern void clearChainTable(void);
extern void eventsInit(void);
extern void doEvlrn(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal);
extern BYTE numEv(unsigned char tableIndex);
extern int getEv(unsigned char tableIndex, unsigned char evNum);
extern unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal);
extern BYTE getEVs(unsigned char tableIndex);
extern unsigned char addEvent(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal, BOOL forceOwnNN);
extern unsigned char removeEvent(WORD nodeNumber, WORD eventNumber);
extern unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal);
extern WORD getNN(unsigned char tableIndex);
extern WORD getEN(unsigned char tableIndex);
extern BOOL validStart(unsigned char tableIndex);
extern void checkRemoveTableEntry(unsigned char tableIndex);

extern rom near EventTable * eventTable;
extern Event producedEvent;
extern BYTE evs[EVperEVT];

// Internal functions

void 	doNnclr(void);
void 	doNnevn(void);
void 	doNerd(void);
void    doNenrd(unsigned char tableIndex);
void 	doRqevn(void);
void	doEvuln(WORD nodeNumber, WORD eventNumber);
void 	doReval(BYTE tableIndex, BYTE evNum);
void 	doReqev(WORD nodeNumber, WORD eventNumber, BYTE evNum);
void 	doEvlrn(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal);
void 	doEvlrni(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal);

BYTE    findEvent( WORD eventNode, WORD eventNum);
BYTE    findEventContinuation(BYTE eventIndex);

BYTE    eventHash( BYTE nodeByte, BYTE eventBYTE );

BOOL    parseCbusEvent( BYTE *msg );

#ifdef HASH_TABLE
extern void rebuildHashtable(void);
#endif



#endif	// __EVENTS_H
