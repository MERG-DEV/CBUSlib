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
 * 
 * 
 * Currently written for:
 *  * XC8 compiler
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

// forward references
void rebuildHashtable(void);
unsigned char getHash(WORD nn, WORD en);
BYTE tableIndexToEvtIdx(BYTE tableIndex);
BYTE evtIdxToTableIndex(BYTE evtIdx);
unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal);

extern void processEvent(unsigned char action, BYTE * msg);

//Events are stored in Flash just below NVs

/*
 * The Event storage.
 */

// EVENTS
//
// The events are stored as a hash table in flash (flash is faster to read than EEPROM)
// There can be up to 255 events. Since the address in the hash table will be 16 bits, and the
// address of the whole event table can also be covered by a 16 bit address, there is no
// advantage in having a separate hashed index table pointing to the actual event table.
// Therefore the hashing algorithm produces the index into the actual event table, which
// can be shifted to give the address - each event table entry is 16 bytes. After the event
// number and hash table overhead, this allows up to 10 EVs per event.
//
// This generic FLiM code needs no knowledge of specific EV usage except that EV#1 is 
// to define the Produced events (if the PRODUCED_EVENTS definition is defined).
//
// BEWARE must set NUM_EVENTS to a maximum of 255!
// If set to 256 then the for (unsigned char i=0; i<NUM_EVENTS; i++) loops will never end
// as they use an unsigned char instead of int for space/performance reasons.
//
// BEWARE Concurrency: The functions which use the eventTable and hash/lookup must not be used
// whilst there is a chance of the functions which modify the eventTable of RAM based 
// hash/lookup tables being called. These functions should therefore either be called
// from the same thread or disable interrupts. 
//
// define HASH_TABLE to use event hash tables for fast access - at the expense of some RAM

typedef union
{
    struct
    {
        BOOL    continues:1;    // there is another entry 
        BOOL    continuation:1; // Continuation of previous event entry
        BOOL    freeEntry:1;    // not used - takes priority over other flags
    };
    BYTE    free;       // Set to 0xFF for free entry, initially set to zero for entry in use, then producer flag set if required.
} EventTableFlags;

typedef struct {
    Event event;    // the NN and EN
    BYTE next;      // index to continuation also indicates if entry is free
    EventTableFlags flags;
    BYTE evs[EVENT_TABLE_WIDTH];
} EventTable;
#ifdef __XC8
const EventTable eventTable[NUM_EVENTS] @AT_EVENTS;
#else
#pragma romdata myEvents=AT_EVENTS
const rom EventTable eventTable[NUM_EVENTS];
#endif
#define NO_INDEX    0xff

 
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
#pragma udata large_event_hash
// the lookup table to find an EventTable entry by Producer action
BYTE action2Event[NUM_PRODUCER_ACTIONS];    // MIO: 66 bytes
#endif
// The hashtable to find the EventTable entry by Event.
// This RAM hash table will probably be more than 256 bytes. With C18 this leads to
// an error:
//Error - section '.udata_events.o' can not fit the section. Section '.udata_events.o' length=0x000002c2
//

BYTE eventChains[HASH_LENGTH][CHAIN_LENGTH];    // MIO: 32*20 bytes = 640
#pragma udata 
#endif

/**
 * eventsInit called during initialisation - initialises event support.
 * Called after power up to initialise RAM.
 */
void eventsInit( void ) {
#ifdef HASH_TABLE
    rebuildHashtable();
#endif
} //eventsInit

/**
 * Clear all Events.
 */
void doNnclr(void) {
    if (flimState == fsFLiMLearn) {
        unsigned char tableIndex;
        for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
            // set the free flag
            writeFlashByte((BYTE*)&(eventTable[tableIndex].flags), 0xff);
        }
        flushFlashImage();
#ifdef HASH_TABLE
        rebuildHashtable();
#endif
    } else {
            cbusMsg[d3] = CMDERR_NOT_LRN;
            cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
 	}
} //doNnclr

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
        if (eventTable[i].flags.freeEntry) {
            count++;
        }
    }
    cbusMsg[d3] = count;
    cbusSendOpcMyNN( 0, OPC_EVNLF, cbusMsg );
} // doNnevn

// Read all stored events
void doNerd(void) {
	//TODO send response OPC_ENRSP (presumably once for each event and EV?  Study FCU interaction with existing modules)
    unsigned char tableIndex;
    for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
        // if its not free and not a continuation then it is start of an event
        if (( ! eventTable[tableIndex].flags.freeEntry) && ( ! eventTable[tableIndex].flags.continuation)) {
            cbusMsg[d3] = eventTable[tableIndex].event.NN>>8;
            cbusMsg[d4] = eventTable[tableIndex].event.NN&0xff;
            cbusMsg[d5] = eventTable[tableIndex].event.EN>>8;
            cbusMsg[d6] = eventTable[tableIndex].event.EN&0xff;
            cbusMsg[d7] = tableIndexToEvtIdx(tableIndex); 
            cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg );   // TODO we need to send the EV# but we can't
        }
    }
} // doNerd

/**
 * Read number of stored events
 * This returns the number of events which is different to the number of used slots 
 * in the Event table.
 */
void doRqevn(void) {
    //Pete's original code kept a counter in EEPROM but here I count the number
    // of used slots.
    unsigned char count = 0;
    unsigned char i;
    for (i=0; i<NUM_EVENTS; i++) {
        if (( ! eventTable[i].flags.freeEntry) && ( ! eventTable[i].flags.continuation)) {
            count++;    
        }
    }
    cbusMsg[d3] = count;
    cbusSendOpcMyNN( 0, OPC_NUMEV, cbusMsg );
} // doRqevn

/**
 * Unlearn event.
 * @param nodeNumber
 * @param eventNumber
 */
void doEvuln(WORD nodeNumber, WORD eventNumber) {
    // need to delete this action from the Event table. 
    unsigned char tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex == NO_INDEX) return; // not found
    // found the event to delete
    // set the free flag
    writeFlashByte((BYTE*)&(eventTable[tableIndex].flags), 0xff);
    // Now follow the next pointer
    while (eventTable[tableIndex].flags.continues) {
        tableIndex = eventTable[tableIndex].next;
        // the continuation flag of this entry should be set but I'm 
        // not going to check as I wouldn't know what to do if it wasn't set
                    
        // set the free flag
        writeFlashByte((BYTE*)&(eventTable[tableIndex].flags), 0xff);
    }
#ifdef HASH_TABLE
    // easier to rebuild from scratch
    rebuildHashtable();
#endif
    flushFlashImage();
    // Don't send a WRACK
}

/**
 * Read an event variable by index.
 * TODO check meaning of EN# Again not clear what the CBUS spec actually needs here as the Index is implementation
 * specific.
 */
void doReval(void) {
	// Get event index and event variable number from message
	// Send response with EV value
    unsigned char tableIndex = evtIdxToTableIndex(cbusMsg[d3]);
    unsigned char evNum = cbusMsg[d4];
    
    // check it is a valid index
    if (tableIndex < NUM_EVENTS) {
        if (( !eventTable[tableIndex].flags.freeEntry) && ( ! eventTable[tableIndex].flags.continuation)) {
            int evVal = getEv(tableIndex, evNum);
            if (evVal >= 0) {
                cbusMsg[5] = getEv(tableIndex, evNum);
                cbusSendOpcMyNN( 0, OPC_NEVAL, cbusMsg );
                return;
            }
            cbusMsg[d3] = CMDERR_INV_EV_IDX;
            cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
        }
    }
    cbusMsg[d3] = CMDERR_INVALID_EVENT;
    cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
} // doReval


/**
 * Read an event variable by event id.
 * TODO check meaning of EN#
 * @param nodeNumber
 * @param eventNumber
 * @param evNum
 */
void doReqev(WORD nodeNumber, WORD eventNumber, BYTE evNum) {
    int evVal;
    // get the event
    unsigned char tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex == NO_INDEX) {
        cbusMsg[d3] = CMDERR_INVALID_EVENT;
        cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
    }
    cbusMsg[d3] = eventNumber >> 8;
    cbusMsg[d4] = eventNumber & 0x00FF;
    cbusMsg[d5] = evNum;
    evVal = getEv(tableIndex, evNum);
    if (evVal >= 0) {
        cbusMsg[d6] = getEv(tableIndex, evNum);
        cbusSendOpcMyNN( 0, OPC_EVANS, cbusMsg);
        return;
    }
    cbusMsg[d3] = CMDERR_INV_EV_IDX;
    cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
}

/**
 * Teach event whilst in learn mode.
 * Teach or reteach an event associated with an action. 
 * The may (optionally) need to create a new event and then optionally
 * create additional chained entries. All newly allocated table entries need
 * to be initialised.
 * @param nodeNumber
 * @param eventNumber
 * @param evNum the EV number
 * @param evVal the EV value
 * @return 
 */
void doEvlrn(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal ) {
    unsigned char tableIndex;
    unsigned char error;
    // evNum starts at 1 - convert to zero based
    if (evNum == 0) {
        cbusMsg[d3] = CMDERR_INV_EV_IDX;
        cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
        return;
    }
    evNum--;
    // do we currently have an event
    tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex != NO_INDEX) {
        // found the event
        error = writeEv(tableIndex, evNum, evVal);
        if (error) {
            // failed to write
            cbusMsg[d3] = CMDERR_INV_EV_IDX;
            cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
            return;
        }
        // success
        flushFlashImage();
#ifdef HASH_TABLE
        rebuildHashtable();
#endif
        cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
        return;
    }
    // event start not found so find an empty slot and create one
    for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
        if (eventTable[tableIndex].flags.freeEntry) {
            unsigned char e;
            // found a free slot, initialise it
            setFlashWord((WORD*)&eventTable[tableIndex].event.NN, nodeNumber);
            setFlashWord((WORD*)&eventTable[tableIndex].event.EN, eventNumber);
            writeFlashByte((BYTE*)&eventTable[tableIndex].flags, 0);
            for (e = 0; e < EVENT_TABLE_WIDTH; e++) {
                writeFlashByte((BYTE*)&eventTable[tableIndex].evs[e], NO_ACTION);
            }
            error = writeEv(tableIndex, evNum, evVal);
            if (error) {
                // failed to write
                cbusMsg[d3] = CMDERR_INV_EV_IDX;
                cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
                return;
            }
            // success
            flushFlashImage();
#ifdef HASH_TABLE
            rebuildHashtable();
#endif
            cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
            return;
        }
    }
}

/**
 * Find an event in the eventTable and return it's index
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
        if (tableIndex == NO_INDEX) return NO_INDEX;
        if ((eventTable[tableIndex].event.NN == nodeNumber) && (eventTable[tableIndex].event.EN == eventNumber)) {
            return tableIndex;
        }
    }
#else
    for (unsigned char tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        if (( ! eventTable[tableIndex].flags.freeEntry) && ( ! eventTable[tableIndex].flags.continuation)) {
            if ((eventTable[tableIndex].event.NN == nodeNumber) && (eventTable[tableIndex].event.EN == eventNumber)) {
                return tableIndex;
            }
        }
    }
#endif
    return NO_INDEX;
}

/**
 * Write an EV value to an event
 * @param tableIndex the index into the event table
 * @param evNum the EV number (zero based)
 * @param evVal
 * @return 0 if success otherwise the error
 */
unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal) {
    if (evNum > EVperEVT) {
        return CMDERR_INV_EV_IDX;
    }
    while (evNum >= EVENT_TABLE_WIDTH) {
        unsigned char nextIdx;
        // skip forward looking for the right chained table entry
        evNum -= EVENT_TABLE_WIDTH;
        
        if (eventTable[tableIndex].flags.continues) {
            tableIndex = eventTable[tableIndex].next;
        } else {
            // find the next free entry
            for (nextIdx = tableIndex+1 ; nextIdx < NUM_EVENTS; nextIdx++) {
                if (eventTable[nextIdx].flags.freeEntry) {
                    unsigned char e;
                     // found a free slot, initialise it
                    setFlashWord((WORD*)&eventTable[nextIdx].event.NN, 0xff); // this field not used
                    setFlashWord((WORD*)&eventTable[nextIdx].event.EN, 0xff); // this field not used
                    writeFlashByte((BYTE*)&eventTable[nextIdx].flags, 1);    // set continuation flag
                    for (e = 0; e < EVENT_TABLE_WIDTH; e++) {
                        writeFlashByte((BYTE*)&eventTable[nextIdx].evs[e], NO_ACTION); // clear the EVs
                    }
                    // set the next of the previous in chain
                    writeFlashByte((BYTE*)&eventTable[tableIndex].next, nextIdx);
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
    writeFlashByte((BYTE*)&eventTable[tableIndex].evs[evNum], evVal);
    return 0;
}
 
/**
 * Return an EV value for an event.
 * @param tableIndex the index of the start of an event
 * @param evNum ev number starts at 1
 * @return the ev value or -1 if error
 */
int getEv(unsigned char tableIndex, unsigned char evNum) {
    if ((eventTable[tableIndex].flags.continuation) || (eventTable[tableIndex].flags.freeEntry)) {
        // not a valid start
        return -1;
    }
    if (evNum == 0) {
        return -1;
    }
    if (evNum > EVperEVT) {
        return -1;
    }
    evNum--;    // make it start from 0
    while (evNum >= EVENT_TABLE_WIDTH) {
        // if evNum is beyond current eventTable entry move to next one
        if (eventTable[tableIndex].flags.continues) {
            tableIndex = eventTable[tableIndex].next;
        } else {
            // beyond last available EV
            return -1;
        }
    }
    // it is within this entry
    return eventTable[tableIndex].evs[evNum];
}

/**
 * Return the NN for an event.
 * Getter so that the application code can obtain information about the event.
 * @param tableIndex the index of the start of an event
 * @return the Node Number
 */
WORD getNN(unsigned char tableIndex) {
    return eventTable[tableIndex].event.NN;
}

/**
 * Return the EN for an event.
 * Getter so that the application code can obtain information about the event.
 * @param tableIndex the index of the start of an event
 * @return the Event Number
 */
WORD getEN(unsigned char tableIndex) {
    return eventTable[tableIndex].event.EN;
}

/**
 * This Consumes a CBUS event if it has been provisioned.
 * @param msg
 * @return 
 */
BOOL parseCbusEvent(BYTE * msg) {
    WORD nodeNumber = (msg[d1] << 8) + msg[d2];
    WORD eventNumber = (msg[d3] << 8) + msg[d4];
    unsigned char tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex != NO_INDEX) {
        processEvent(tableIndex, msg);
        return TRUE;
    }
    return FALSE;
} 
 
/*
 * The CBUS spec uses "EN#" as an index into an "Event Table". This si very implementation
 * specific. In this implementation we do actually have an event table behind the scenes
 * so we can have an EN#. However we may also wish to provide some kind of mapping between
 * the CBUS index and out actual implementation specific index. These functions allow us
 * to have a mapping.
 * I currently I just adjust by 1 since the CBUS index starts at 1 whilst the eventTable
 * index starts at 0.
 */
/**
 * Convert an evtIdx from CBUS to an index into the eventTable.
 * This implements a 1:1 mapping 
 * @param evtIdx
 * @return an index into eventTable
 */
BYTE evtIdxToTableIndex(BYTE evtIdx) {
    return evtIdx-1;
}

/**
 * Convert an internal tableIndex into a CBUS EvtIdx.
 * This implements a 1:1 mapping 
 * @param tableIndex index into the eventTable
 * @return an CBUS EvtIdx
 */
BYTE tableIndexToEvtIdx(BYTE tableIndex) {
    return tableIndex+1;
}

/**
 * Delete all occurrences of the action.
 * @param action
 */
void deleteAction(unsigned char action) {
    if (action != NO_ACTION) {
        unsigned char tableIndex;
        for (tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
            unsigned char e;
            for (e=0; e<EVENT_TABLE_WIDTH; e++) {
                if (( ! eventTable[tableIndex].flags.freeEntry) && ( ! eventTable[tableIndex].flags.continuation)) {
                    if (eventTable[tableIndex].evs[e] == action) {
                        writeEv(tableIndex, 0, NO_ACTION);
                    }
                }
            }
        }
#ifdef HASH_TABLE
        rebuildHashtable();                
#endif
    }
}

#ifdef PRODUCED_EVENTS
/**
 * Get the Produced Event to transmit for the specified action.
 * If the same produced action has been provisioned for more than 1 event
 * only the first provisioned event will be returned.
 * 
 * @param action the produced action
 * @return the produced event or NULL if none has been provisioned
 */ 
const Event * getProducedEvent(unsigned char action) {
     if (action >= NUM_PRODUCER_ACTIONS) return NULL;    // not a produced valid action
#ifdef HASH_TABLE
     if (action2Event[action] == NO_ACTION) return NULL;
     return &eventTable[action2Event[action]].event;
#else
    for (unsigned char tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        if (( ! eventTable[tableIndex].flags.freeEntry) && ( ! eventTable[tableIndex].flags.continuation)) {
            if (eventTable[tableIndex].evs[0] == action) {
                return &eventTable[tableIndex];
            }
        }
    }
    return NULL,
#endif
}
#endif

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
#ifdef PRODUCED_EVENTS
    // first initialise to nothing
    unsigned char action;
    for (action=0; action<NUM_PRODUCER_ACTIONS; action++) {
        action2Event[action] = NO_ACTION;
    }
#endif
    for (hash=0; hash<HASH_LENGTH; hash++) {
        for (chainIdx=0; chainIdx < CHAIN_LENGTH; chainIdx++) {
            eventChains[hash][chainIdx] = NO_INDEX;
        }
    }
    // now scan the event2Action table and populate the hash and lookup tables
    
    for (tableIndex=0; tableIndex<NUM_EVENTS; tableIndex++) {
        if (( ! eventTable[tableIndex].flags.freeEntry) && ( ! eventTable[tableIndex].flags.continuation)) {
            // found the start of an event definition
#ifdef PRODUCED_EVENTS
            // EV#1 is used to store the Produced event's action
            int action = getEv(tableIndex, 1);
            if ((action > 0) && (action <= NUM_PRODUCER_ACTIONS)) {
                action2Event[action] = tableIndex;
            }
#endif
            // The other EVs are for the consumed events
            hash = getHash(eventTable[tableIndex].event.NN, eventTable[tableIndex].event.EN);
            
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

