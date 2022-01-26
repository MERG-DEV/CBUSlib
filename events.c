/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 Routines for CBUS event management - part of CBUS libraries for PIC 18F

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


	These event routines have no code or definitions that are specific to any
	module, so they can be used to provide event facilities for any module 
	using these libraries.
	
 History for this file:
	28/12/15	Pete Brownlow	- Factored out from FLiM module
    25/2/16     Pete Brownlow   - coding in progress
    23/5/17     Ian Hogg        - added support for produced events
    05/06/21    Ian Hogg        - removed happeninga and actions so that this file just handles EV bytes
 * 
 * 
 * Currently written for:
 *  * C18 compiler
 * 
 * This file used the following PIC peripherals:
 *  * none
 * 
 */
#include <stddef.h>
#include "module.h"
#include "FliM.h"
#include "events.h"
#include "cbus.h"
#include "romops.h"
#ifdef TIMED_RESPONSE
#include "timedResponse.h"
#endif


// forward references
void rebuildHashtable(void);
unsigned char getHash(WORD nn, WORD en);
BYTE tableIndexToEvtIdx(BYTE tableIndex);
BYTE evtIdxToTableIndex(BYTE evtIdx);
void checkRemoveTableEntry(unsigned char tableIndex);
unsigned char removeTableEntry(unsigned char tableIndex);
unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal);
WORD getNN(unsigned char tableIndex);
WORD getEN(unsigned char tableIndex);

extern void processEvent(unsigned char action, BYTE * msg);

//Events are stored in Flash just below NVs

/** Event handling.
 *
 * The events are stored as a hash table in flash (flash is faster to read than EEPROM)
 * There can be up to 255 events. Since the address in the hash table will be 16 bits, and the
 * address of the whole event table can also be covered by a 16 bit address, there is no
 * advantage in having a separate hashed index table pointing to the actual event table.
 * Therefore the hashing algorithm produces the index into the actual event table, which
 * can be shifted to give the address - each event table entry is 16 bytes. After the event
 * number and hash table overhead, this allows up to 10 EVs per event.
 *
 * This generic FLiM code needs no knowledge of specific EV usage except that EV#1 is 
 * to define the Produced events (if the PRODUCED_EVENTS definition is defined).
 *
 * BEWARE must set NUM_EVENTS to a maximum of 255!
 * If set to 256 then the for (unsigned char i=0; i<NUM_EVENTS; i++) loops will never end
 * as they use an unsigned char instead of int for space/performance reasons.
 *
 * BEWARE Concurrency: The functions which use the eventTable and hash/lookup must not be used
 * whilst there is a chance of the functions which modify the eventTable of RAM based 
 * hash/lookup tables being called. These functions should therefore either be called
 * from the same thread or disable interrupts. 
 *
 * define HASH_TABLE to use event hash tables for fast access - at the expense of some RAM
 *
 * The code in event.c is responsible for storing EVs for each defined event and 
 * also for allowing speedy lookup of EVs given an Event or finding an Event given 
 * a Happening which is stored in EV#1.
 * 
 * Events are stored in the EventTable which consists of rows containing the following 
 * fields:
 * * EventTableFlags flags         1 byte
 * * BYTE next                     1 byte
 * * Event event                   4 bytes
 * * BYTE evs[EVENT_TABLE_WIDTH]   EVENT_TABLE_WIDTH bytes
 * 
 * The number of table entries is defined by NUM_EVENTS.
 * 
 * The EVENT_TABLE_WIDTH determines the number of EVs that can be stored within 
 * a single EventTable row. Where events can have more EVs that can be fitted within 
 * a single row then multiple rows are chained together using the 'next' field. 
 * 'next' indicates the index into the EventTable for chained entries for a single 
 * Event.
 * 
 * An example to clarify is the CANMIO which sets EVENT_TABLE_WITH to 10 so that 
 * size of a row is 16bytes. A chain of two rows can store 20 EVs. CANMIO has a 
 * limit of 20 EVs per event (EVperEVT) so that a maximum of 2 entries are chained.
 * 
 * The 'event' field is only used in the first in a chain of entries and contains 
 * the NN/EN of the event.
 * 
 * The EventTableFlags have the following entries:
 * eVsUsed                4 bits
 * continued                       1 bit
 * forceOwnNN                      1 bit
 * freeEntry                       1 bit
 * 
 * The 'continued' flag indicates if there is another table entry chained to this 
 * one. If the flag is set then the 'next' field contains the index of the chained 
 * entry.
 * 
 * The 'forceOwnNN' flag indicates that for produced events the NN in the event 
 * field should be ignored and the module's current NN should be used instead. 
 * This allows default events to be maintained even if the NN of the module is 
 * changed. Therefore this flag should be set for default produced events.
 * 
 * The 'freeEntry' flag indicates that this entry in the EventTable is currently 
 * unused.
 * 
 * The 'eVsUsed' field records how many of the evs contain valid data. 
 * It is only applicable for the last entry in the chain since all EVs less than 
 * this are assumed to contain valid data. Since this field is only 4 bits long 
 * this places a limit on the EVENT_TABLE_WIDTH of 15.
 * 
 * EXAMPLE
 * Let's go through an example of filling in the table. We'll look at the first 
 * 4 entries in the table and let's have EVENT_TABLE_WIDTH=4 but have EVperEVT=8.
 * 
 * At the outset there is an empty table. All rows have the 'freeEntry' bit set:
 * 
 * index   eVsUsed        continued       forceOwnNN      freeEntry       next    Event   evs[]
 * 0       0              0               0               1               0       0:0     0,0,0,0
 * 1       0              0               0               1               0       0:0     0,0,0,0
 * 2       0              0               0               1               0       0:0     0,0,0,0
 * 3       0              0               0               1               0       0:0     0,0,0,0
 *
 * Now if an EV of an event is set (probably using EVLRN CBUS command) then the 
 * table is updated. Let's set EV#1 for event 256:101 to the value 99:
 * 
 * index   eVsUsed        continued       forceOwnNN      freeEntry       next    Event   evs[]
 * 0       1              0               0               0               0       256:101 99,0,0,0
 * 1       0              0               0               1               0       0:0     0,0,0,0
 * 2       0              0               0               1               0       0:0     0,0,0,0
 * 3       0              0               0               1               0       0:0     0,0,0,0
 * 
 * Now let's set EV#2 of event 256:102 to 15:
 * 
 * index   eVsUsed        continued       forceOwnNN      freeEntry       next    Event   evs[]
 * 0       1              0               0               0               0       256:101 99,0,0,0
 * 1       2              0               0               0               0       256:102 0,15,0,0
 * 2       0              0               0               1               0       0:0     0,0,0,0
 * 3       0              0               0               1               0       0:0     0,0,0,0
 * 
 * Now let's set EV#8 of event 256:101 to 29:
 * 
 * index   eVsUsed        continued       forceOwnNN      freeEntry       next    Event   evs[]
 * 0       1              1               0               0               0       256:101 99,0,0,0
 * 1       2              0               0               0               0       256:102 0,15,0,0
 * 2       4              0               0               0               0       0:0     0,0,0,29
 * 3       0              0               0               1               0       0:0     0,0,0,0
 * 
 * To perform the speedy lookup of EVs given an Event a hash table can be used by 
 * defining HASH_TABLE. The hash table is stored in 
 * BYTE eventChains[HASH_LENGTH][CHAIN_LENGTH];
 * 
 * An event hashing function is provided BYTE getHash(nn, en) which should give 
 * a reasonable distribution of hash values given the typical events used.
 * 
 * This table is populated from the EventTable upon power up using rebuildHashtable(). 
 * This function must be called before attempting to use the hash table. Each Event 
 * from the EventTable is hashed using getHash(nn,en), trimmed to the HASH_LENGTH 
 * and the index in the EventTable is then stored in the eventChains at the next 
 * available bucket position.
 * 
 * When an Event is received from CBUS and we need to find its index within the 
 * EventTable it is firstly hashed using getHash(nn,en), trimmed to HASH_LENGTH 
 * and this is used as the first index into eventChains[][]. We then step through 
 * the second index of buckets within the chain. Each entry is an index into the 
 * eventTable and the eventTable's event field is checked to see if it matches the
 * received event. It it does match then the index into eventTable has been found 
 * and is returned. The EVs can then be accessed from the ev[] field.
 * 
 * If PRODUCED_EVENTS is defined in addition to HASH_TABLE then an additional 
 * lookup table BYTE action2Event[NUM_HAPPENINGS] is used to obtain an Event 
 * using a Happening (previously called Actions) stored in EV#1. This table is 
 * also populated using rebuildHashTable(). Given a Happening this table can be 
 * used to obtain the index into the EventTable for the Happening so the Event 
 * at that index in the EventTable can be transmitted onto the CBUS.
 */

#ifdef __XC8
const EventTable * eventTable =(EventTable *)AT_EVENTS;
#else
//#pragma romdata myEvents=AT_EVENTS
//volatile near rom EventTable eventTable[NUM_EVENTS];
rom near EventTable * eventTable = (rom near EventTable*)AT_EVENTS;
#endif


 
#ifdef HASH_TABLE
/*
 * HASH TABLE CODE
 *
 * We have 2 lookup/hash tables.
 * 1) Look up an eventTable entry by Event - used for consumed events.
 * 2) Look up an Event by Produced Action - used for produced events.
 * 
 * These are stored in RAM.
 */

#ifdef PRODUCED_EVENTS
#ifdef __C18
#pragma udata large_event_hash
#endif
// the lookup table to find an EventTable entry by Happening
BYTE happening2Event[NUM_HAPPENINGS];    // MIO: 64+8 bytes
#endif
// The hashtable to find the EventTable entry by Event.
// This RAM hash table will probably be more than 256 bytes. With C18 this leads to
// an error:
//Error - section '.udata_events.o' can not fit the section. Section '.udata_events.o' length=0x000002c2
//

BYTE eventChains[HASH_LENGTH][CHAIN_LENGTH];    // MIO: 32*20 bytes = 640
#ifdef __C18
#pragma udata
#endif
#endif

/**
 * eventsInit called during initialisation - initialises event support.
 * Called after power up to initialise RAM.
 */
void eventsInit( void ) {
#ifdef HASH_TABLE
    // nodeId must already to have been initialised. 
    // Therefore make sure cbusInit has already been called
    rebuildHashtable();
#endif
} //eventsInit

/**
 * Checks if the specified index is the start of a set of linked entries.
 * 
 * @param tableIndex the index into eventtable to check
 * @return true if the specified index is the start of a linked set
 */
BOOL validStart(unsigned char tableIndex) {
    EventTableFlags f;
#ifdef SAFETY
    if (tableIndex >= NUM_EVENTS) return FALSE;
#endif
    f.asByte = readFlashBlock((WORD)(& (eventTable[tableIndex].flags.asByte)));
    if (( !f.freeEntry) && ( ! f.continuation)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * Removes all events including default events.
 */
void clearAllEvents(void) {
    unsigned char tableIndex;
    for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
        // set the free flag
        writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), 0xff);
    }
    flushFlashImage();
#ifdef HASH_TABLE
    rebuildHashtable();
#endif
}

/**
 * Read number of available event slots.
 * This returned the number of unused slots in the Consumed event Event2Action table.
 */
void doNnevn(void) {
    //Pete's original code kept a counter in EEPROM but here I count the number
    // of unused slots.
    unsigned char count = 0;
    unsigned char i;
    for (i=0; i<NUM_EVENTS; i++) {
        EventTableFlags f;
        f.asByte = readFlashBlock((WORD)(& (eventTable[i].flags.asByte)));
        if (f.freeEntry) {
            count++;
        }
    }
    cbusMsg[d3] = count;
    cbusSendOpcMyNN( 0, OPC_EVNLF, cbusMsg );
} // doNnevn

#ifdef TIMED_RESPONSE
/**
 * Do the NERD. 
 * This sets things up so that timedResponse will do the right stuff.
 */
void doNerd(void) {
    timedResponse = TIMED_RESPONSE_NERD;
    timedResponseStep = 0;
}
#else
/**
 * Handle a NERD request by returning a response for each event.
 * 
 */
void doNerd(void) {
    unsigned char tableIndex;
    for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
        // if its not free and not a continuation then it is start of an event
        if (validStart(tableIndex)) {
            WORD n = getNN(tableIndex);
            cbusMsg[d3] = n >> 8;
            cbusMsg[d4] = n & 0xFF;
            
            n = getEN(tableIndex);
            cbusMsg[d5] = n >> 8;
            cbusMsg[d6] = n & 0xFF;
            
            cbusMsg[d7] = tableIndexToEvtIdx(tableIndex); 
            while ( !cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg ))
                ;   // Busy wait trying to send
        }   
    }
} // doNerd
#endif

/**
 * Read a single stored event by index and return a ENRSP response.
 * 
 * @param index index into event table
 */
void doNenrd(unsigned char index) {
    unsigned char tableIndex;
    WORD n;
    
    tableIndex = evtIdxToTableIndex(index);
    // check this is a valid index
    if ( ! validStart(tableIndex)) {
        doError(CMDERR_INVALID_EVENT);
            // DEBUG  TODO remove
//        cbusMsg[d7] = readFlashBlock((WORD)(& (eventTable[tableIndex].flags.asByte))); 
//        cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg );
        return;
    }
    n = getNN(tableIndex);
    cbusMsg[d3] = n >> 8;
    cbusMsg[d4] = n & 0xFF;
            
    n = getEN(tableIndex);
    cbusMsg[d5] = n >> 8;
    cbusMsg[d6] = n & 0xFF;
            
    cbusMsg[d7] = index; 
    cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg );

} // doNenrd

/**
 * Read number of stored events.
 * This returns the number of events which is different to the number of used slots 
 * in the Event table.
 */
void doRqevn(void) {
    //Pete's original code kept a counter in EEPROM but here I count the number
    // of used slots.
    unsigned char count = 0;
    unsigned char i;
    for (i=0; i<NUM_EVENTS; i++) {
        if (validStart(i)) {
            count++;    
        }
    }
    cbusMsg[d3] = count;
    cbusSendOpcMyNN( 0, OPC_NUMEV, cbusMsg );
} // doRqevn



/**
 * Remove event.
 * 
 * @param nodeNumber
 * @param eventNumber
 * @return error or 0 for success
 */
unsigned char removeEvent(WORD nodeNumber, WORD eventNumber) {
    // need to delete this action from the Event table. 
    unsigned char tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex == NO_INDEX) return CMDERR_INV_EV_IDX; // not found
    // found the event to delete
    return removeTableEntry(tableIndex);
}

unsigned char removeTableEntry(unsigned char tableIndex) {
    EventTableFlags f;

#ifdef SAFETY
    if (tableIndex >= NUM_EVENTS) return CMDERR_INV_EV_IDX;
#endif
    if (validStart(tableIndex)) {
        // set the free flag
        writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), 0xff);
        // Now follow the next pointer
        f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
        while (f.continued) {
            tableIndex = readFlashBlock((WORD)(& eventTable[tableIndex].next));
            f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
        
            if (tableIndex >= NUM_EVENTS) return CMDERR_INV_EV_IDX; // shouldn't be necessary
        
            // the continuation flag of this entry should be set but I'm 
            // not going to check as I wouldn't know what to do if it wasn't set
                    
            // set the free flag
            writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), 0xff);
        
        }
        flushFlashImage();
#ifdef HASH_TABLE
        // easier to rebuild from scratch
        rebuildHashtable();
#endif
    }
    return 0;
}
/**
 * Add an event/EV.
 * Teach or re-teach an EV for an event. 
 * This may (optionally) need to create a new event and then optionally
 * create additional chained entries. All newly allocated table entries need
 * to be initialised.
 * 
 * @param nodeNumber
 * @param eventNumber
 * @param evNum the EV index (starts at 0 for the produced action)
 * @param evVal the EV value
 * @param forceOwnNN the value of the flag
 * @return error number or 0 for success
 */
unsigned char addEvent(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal, BOOL forceOwnNN) {
    unsigned char tableIndex;
    unsigned char error;
    // do we currently have an event
    tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex == NO_INDEX) {
        // Ian - 2k check for special case. Don't create an entry for a NO_ACTION
        // This is a solution to the problem of FCU filling the event table with unused
        // 00 Actions. 
        // It does not fix a much less frequent problem of releasing some of the 
        // table entries when they are filled with No Action.
        if (evVal == EV_FILL) {
            return 0;
        }
        error = 1;
        // didn't find the event so find an empty slot and create one
        for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
            EventTableFlags f;
            f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
            if (f.freeEntry) {
                unsigned char e;
                // found a free slot, initialise it
                setFlashWord((WORD*)&(eventTable[tableIndex].event.NN), nodeNumber);
                setFlashWord((WORD*)&(eventTable[tableIndex].event.EN), eventNumber);
            
                f.asByte = 0;
                f.forceOwnNN = forceOwnNN;
                writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), f.asByte);
            
                for (e = 0; e < EVENT_TABLE_WIDTH; e++) {
                    writeFlashByte((BYTE*)&(eventTable[tableIndex].evs[e]), EV_FILL);
                }
                flushFlashImage();
#ifdef HASH_TABLE
                rebuildHashtable();
#endif
                error = 0;
                break;
            }
        }
        if (error) {
            return CMDERR_TOO_MANY_EVENTS;
        }
    }
 
    if (writeEv(tableIndex, evNum, evVal)) {
        // failed to write
        return CMDERR_INV_EV_IDX;
    }
    // success
    flushFlashImage();
#ifdef HASH_TABLE
    rebuildHashtable();
#endif
    return 0;
}

/**
 * Find an event in the eventTable and return it's index.
 * 
 * @param nodeNumber
 * @param eventNumber
 * @return index into eventTable or NO_INDEX if not present
 */
unsigned char findEvent(WORD nodeNumber, WORD eventNumber) {
#ifdef HASH_TABLE
    unsigned char hash = getHash(nodeNumber, eventNumber);
    unsigned char chainIdx;
    for (chainIdx=0; chainIdx<CHAIN_LENGTH; chainIdx++) {
        unsigned char tableIndex = eventChains[hash][chainIdx];
        WORD nn, en;
        if (tableIndex == NO_INDEX) return NO_INDEX;
        nn = getNN(tableIndex);
        en = getEN(tableIndex);
        if ((nn == nodeNumber) && (en == eventNumber)) {
            return tableIndex;
        }
    }
#else
    unsigned char tableIndex;
    for (tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        EventTableFlags f;
        f.asByte = readFlashBlock((WORD)(& eventTable[tableIndex].flags));
        if (( ! f.freeEntry) && ( ! f.continuation)) {
            WORD nn, en;
            nn = getNN(tableIndex);
            en = getEN(tableIndex);
            if ((nn == nodeNumber) && (en == eventNumber)) {
                return tableIndex;
            }
        }
    }
#endif
    return NO_INDEX;
}

/**
 * Write an EV value to an event.
 * 
 * @param tableIndex the index into the event table
 * @param evNum the EV number (0 for the produced action)
 * @param evVal
 * @return 0 if success otherwise the error
 */
unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal) {
    EventTableFlags f;
    unsigned char startIndex = tableIndex;
    if (evNum >= EVperEVT) {
        return CMDERR_INV_EV_IDX;
    }
    while (evNum >= EVENT_TABLE_WIDTH) {
        unsigned char nextIdx;
        
        // skip forward looking for the right chained table entry
        evNum -= EVENT_TABLE_WIDTH;
        f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
        
        if (f.continued) {
            tableIndex = readFlashBlock((WORD)(&(eventTable[tableIndex].next)));
            if (tableIndex == NO_INDEX) {
                return CMDERR_INVALID_EVENT;
            }
        } else {
            // Ian - 2k check for special case. Don't create an entry for a NO_ACTION
            // This is a solution to the problem of FCU filling the event table with unused
            // 00 Actions. 
            // It does not fix a much less frequent problem of releasing some of the 
            // table entries when they are filled with No Action.
            // don't add a new table slot just to store a NO_ACTION
            if (evVal == EV_FILL) {
                return 0;
            }
            // find the next free entry
            for (nextIdx = tableIndex+1 ; nextIdx < NUM_EVENTS; nextIdx++) {
                EventTableFlags nextF;
                nextF.asByte = readFlashBlock((WORD)(&(eventTable[nextIdx].flags.asByte)));
                if (nextF.freeEntry) {
                    unsigned char e;
                     // found a free slot, initialise it
                    setFlashWord((WORD*)&(eventTable[nextIdx].event.NN), 0xffff); // this field not used
                    setFlashWord((WORD*)&(eventTable[nextIdx].event.EN), 0xffff); // this field not used
                    writeFlashByte((BYTE*)&(eventTable[nextIdx].flags.asByte), 0x20);    // set continuation flag, clear free and numEV to 0
                    for (e = 0; e < EVENT_TABLE_WIDTH; e++) {
                        writeFlashByte((BYTE*)&(eventTable[nextIdx].evs[e]), EV_FILL); // clear the EVs
                    }
                    // set the next of the previous in chain
                    writeFlashByte((BYTE*)&(eventTable[tableIndex].next), nextIdx);
                    // set the continued flag
                    f.continued = 1;
                    writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), f.asByte);
                    tableIndex = nextIdx;
                    break;
                }
            }
            if (nextIdx >= NUM_EVENTS) {
                // ran out of table entries
                return CMDERR_TOO_MANY_EVENTS;
            }
        } 
    }
    // now write the EV
    writeFlashByte((BYTE*)&(eventTable[tableIndex].evs[evNum]), evVal);
    // update the number per row count
    f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
    if (f.eVsUsed <= evNum) {
        f.eVsUsed = evNum+1;
        writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), f.asByte);
    }
    // If we are deleting then see if we can remove all
    if (evVal == EV_FILL) {
        checkRemoveTableEntry(startIndex);
    }
    return 0;
}
 
/**
 * Return an EV value for an event.
 * 
 * @param tableIndex the index of the start of an event
 * @param evNum ev number starts at 0 (produced)
 * @return the ev value or -error code if error
 */
int getEv(unsigned char tableIndex, unsigned char evNum) {
    EventTableFlags f;
    if ( ! validStart(tableIndex)) {
        // not a valid start
        return -CMDERR_INVALID_EVENT;
    }
    if (evNum >= EVperEVT) {
        return -CMDERR_INV_EV_IDX;
    }
    f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
    while (evNum >= EVENT_TABLE_WIDTH) {
        // if evNum is beyond current eventTable entry move to next one
        if (! f.continued) {
            return -CMDERR_NO_EV;
        }
        tableIndex = readFlashBlock((WORD)(&(eventTable[tableIndex].next)));
        if (tableIndex == NO_INDEX) {
            return -CMDERR_INVALID_EVENT;
        }
        f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
        evNum -= EVENT_TABLE_WIDTH;
    }
    if (evNum+1 > f.eVsUsed) {
        return -CMDERR_NO_EV;
    }
    // it is within this entry
    return readFlashBlock((WORD)(&(eventTable[tableIndex].evs[evNum])));
}

/**
 * Return the number of EVs for an event.
 * 
 * @param tableIndex the index of the start of an event
 * @return the number of EVs
 */
BYTE numEv(unsigned char tableIndex) {
    EventTableFlags f;
    BYTE num=0;
    if ( ! validStart(tableIndex)) {
        // not a valid start
        return 0;
    }
    f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
    while (f.continued) {
        tableIndex = readFlashBlock((WORD)(&(eventTable[tableIndex].next)));
        if (tableIndex == NO_INDEX) {
            return 0;
        }
        f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
        num += EVENT_TABLE_WIDTH;
    }
    num += f.eVsUsed;
    return num;
}

/**
 * Return all the EV values for an event. EVs are put into the global evs array.
 * 
 * @param tableIndex the index of the start of an event
 * @return the error code or 0 for no error
 */
BYTE evs[EVperEVT];
BYTE getEVs(unsigned char tableIndex) {
    EventTableFlags f;
    unsigned char evNum;
    
    if ( ! validStart(tableIndex)) {
        // not a valid start
        return CMDERR_INVALID_EVENT;
    }
    for (evNum=0; evNum < EVperEVT; ) {
        unsigned char evIdx;
        for (evIdx=0; evIdx < EVENT_TABLE_WIDTH; evIdx++) {
            evs[evNum] = readFlashBlock((WORD)(&(eventTable[tableIndex].evs[evIdx])));
            evNum++;
        }
        f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
        if (! f.continued) {
            for (; evNum < EVperEVT; evNum++) {
                evs[evNum] = EV_FILL;
            }
            return 0;
        }
        tableIndex = readFlashBlock((WORD)(&(eventTable[tableIndex].next)));
        if (tableIndex == NO_INDEX) {
            return CMDERR_INVALID_EVENT;
        }
    }
    return 0;
}

/**
 * Return the NN for an event.
 * Getter so that the application code can obtain information about the event.
 * 
 * @param tableIndex the index of the start of an event
 * @return the Node Number
 */
WORD getNN(unsigned char tableIndex) {
    WORD hi;
    WORD lo;
    EventTableFlags f;
    
    f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
    if (f.forceOwnNN) {
        return nodeID;
    }
    lo=readFlashBlock((WORD)(&(eventTable[tableIndex].event.NN)));
    hi = readFlashBlock((WORD)(&(eventTable[tableIndex].event.NN))+1);
    return lo | (hi << 8);
}

/**
 * Return the EN for an event.
 * Getter so that the application code can obtain information about the event.
 * 
 * @param tableIndex the index of the start of an event
 * @return the Event Number
 */
WORD getEN(unsigned char tableIndex) {
    WORD hi;
    WORD lo;
    
    lo=readFlashBlock((WORD)(&(eventTable[tableIndex].event.EN)));
    hi = readFlashBlock((WORD)(&(eventTable[tableIndex].event.EN))+1);
    return lo | (hi << 8);

}

/**
 * This Consumes a CBUS event if it has been provisioned.
 * 
 * @param msg
 * @return 
 */
BOOL parseCbusEvent(BYTE * msg) {
    WORD nodeNumber;
    WORD eventNumber;
    unsigned char tableIndex;
    
    if (msg[d0] & EVENT_SHORT_MASK) {
        nodeNumber = 0;
    } else {
        nodeNumber = msg[d1];
        nodeNumber = nodeNumber << 8;
        nodeNumber |= msg[d2];
    }
    
    eventNumber = msg[d3];
    eventNumber = eventNumber << 8;
    eventNumber |= msg[d4];
    tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex != NO_INDEX) {
        processEvent(tableIndex, msg);
        return TRUE;
    }
    return FALSE;
} 
 
/**
 * Convert an evtIdx from CBUS to an index into the eventTable.
 * The CBUS spec uses "EN#" as an index into an "Event Table". This is very implementation
 * specific. In this implementation we do actually have an event table behind the scenes
 * so we can have an EN#. However we may also wish to provide some kind of mapping between
 * the CBUS index and out actual implementation specific index. These functions allow us
 * to have a mapping.
 * I currently I just adjust by 1 since the CBUS index starts at 1 whilst the eventTable
 * index starts at 0.
 * 
 * @param evtIdx
 * @return an index into eventTable
 */
BYTE evtIdxToTableIndex(BYTE evtIdx) {
    return evtIdx-1;
}

/**
 * Convert an internal tableIndex into a CBUS EvtIdx.
 * 
 * @param tableIndex index into the eventTable
 * @return an CBUS EvtIdx
 */
BYTE tableIndexToEvtIdx(BYTE tableIndex) {
    return tableIndex+1;
}


/**
 * Check to see if any event entries can be removed.
 * 
 * @param tableIndex
 */
void checkRemoveTableEntry(unsigned char tableIndex) {
    unsigned char e;
    
    if ( validStart(tableIndex)) {
        if (getEVs(tableIndex)) {
            return;
        }
        for (e=0; e<EVperEVT; e++) {
            if (evs[e] != EV_FILL) {
                return;
            }
        }
        removeTableEntry(tableIndex);
    }
}

#ifdef HASH_TABLE
/**
 * Obtain a hash for the specified Event. 
 * 
 * The hash table uses an algorithm of an XOR of all the bytes with appropriate shifts.
 * 
 * If we used just the node number, then all short events would hash to the same number
 * If we used just the event number, then for long events the vast majority would 
 * be 1 to 8, so not giving a very good spread.
 *
 * This algorithm hopefully produces a good spread for layouts with either
 * predominantly short events, long events or a mixture.
 * This also means that for layouts using the default FLiM node numbers from 256, 
 * we are effectively starting from zero as far as the hash algorithm is concerned.
 * 
 * @param e the event
 * @return the hash
 */
 unsigned char getHash(WORD nn, WORD en) {
    unsigned char hash;
    // need to hash the NN and EN to a uniform distribution across HASH_LENGTH
    hash = nn ^ (nn >> 8);
    hash = 7*hash + (en ^ (en>>8)); 
    // ensure it is within bounds of eventChains
    hash %= HASH_LENGTH;
    return hash;
}
 
/**
 * Initialise the RAM hash chain for reverse lookup of event to action. Uses the
 * data from the Flash Event2Action table.
 */
void rebuildHashtable(void) {
    // invalidate the current hash table
    unsigned char hash;
    unsigned char chainIdx;
    unsigned char tableIndex;
    int a;
#ifdef PRODUCED_EVENTS
    // first initialise to nothing
    HAPPENING_T happening;
    for (happening=0; happening<NUM_HAPPENINGS; happening++) {
        happening2Event[happening] = NO_INDEX;
    }
#endif
    for (hash=0; hash<HASH_LENGTH; hash++) {
        for (chainIdx=0; chainIdx < CHAIN_LENGTH; chainIdx++) {
            eventChains[hash][chainIdx] = NO_INDEX;
        }
    }
    // now scan the event2Action table and populate the hash and lookup tables
    
    for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
        if (validStart(tableIndex)) {
            unsigned char e;
    
            // found the start of an event definition
#ifdef PRODUCED_EVENTS
            // ev[0] is used to store the Produced event's action
            a = getEv(tableIndex, 0);
            if (a >= 0) {
                happening = a;
                if ((happening >= HAPPENING_BASE) && (happening-HAPPENING_BASE< NUM_HAPPENINGS)) {
                    happening2Event[happening-HAPPENING_BASE] = tableIndex;
                }
            }
#endif
            hash = getHash(getNN(tableIndex), getEN(tableIndex));
                
            for (chainIdx=0; chainIdx<CHAIN_LENGTH; chainIdx++) {
                if (eventChains[hash][chainIdx] == NO_INDEX) {
                    // available
                    eventChains[hash][chainIdx] = tableIndex;
                    break;
                }
            }
        }
    }
}

#endif

