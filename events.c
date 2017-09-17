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
#include "actionQueue.h"

// forward references
void rebuildHashtable(void);
unsigned char getHash(WORD nn, WORD en);
BYTE tableIndexToEvtIdx(BYTE tableIndex);
BYTE evtIdxToTableIndex(BYTE evtIdx);
unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal);
WORD getNN(unsigned char tableIndex);
WORD getEN(unsigned char tableIndex);

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

#ifdef __XC8
const EventTable eventTable[NUM_EVENTS] @AT_EVENTS;
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
#pragma udata large_event_hash
// the lookup table to find an EventTable entry by Producer action
BYTE action2Event[NUM_PRODUCER_ACTIONS];    // MIO: 64+8 bytes
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

BOOL validStart(unsigned char tableIndex) {
    EventTableFlags f;
    f.asByte = readFlashBlock((WORD)(& (eventTable[tableIndex].flags.asByte)));
    if (( !f.freeEntry) && ( ! f.continuation)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


void clearAllEvents() {
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

// Read all stored events
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

// Read a single stored event by index
void doNenrd(unsigned char index) {
    unsigned char tableIndex;
    WORD n;
    
    tableIndex = evtIdxToTableIndex(index);
    // check this is a valid index
    if ( ! validStart(tableIndex)) {
        doError(CMDERR_INVALID_EVENT);
            // DEBUG  TODO remove
        cbusMsg[d7] = readFlashBlock((WORD)(& (eventTable[tableIndex].flags.asByte))); 
        cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg );
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
    
    // DEBUG TODO remove
    cbusMsg[d7] = readFlashBlock((WORD)(& (eventTable[tableIndex].flags.asByte))); 
    cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg );
} // doNenrd

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
        if (validStart(i)) {
            count++;    
        }
    }
    cbusMsg[d3] = count;
    cbusSendOpcMyNN( 0, OPC_NUMEV, cbusMsg );
} // doRqevn

/**
 * Indicates a "request" event using the full event number of 4 bytes. (long event)
 * A request event is used to elicit a status response from a producer when it is required to
 * know the ?state? of the producer without producing an ON or OFF event and to trigger an
 * event from a "combi" node.
 * Only accessible in Setup mode.
 * Does not support default events i.e. those not stored in the event table.
 * @param nodeNumber
 * @param eventNumber
 */
void    doAreq(WORD nodeNumber, WORD eventNumber) {
    PRODUCER_ACTION_T paction;
    int ev0;
    
    unsigned char tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex == NO_INDEX) return;
    // found the matching event
    ev0 = getEv(tableIndex, 0); // get the producer action
    if (ev0 < 0) {
        doError(-ev0);
        return;
    }
    paction = ev0;
    if ((paction >= ACTION_PRODUCER_BASE) && (paction-ACTION_PRODUCER_BASE< NUM_PRODUCER_ACTIONS)) {
        unsigned char bit = paction & 0x7;
        unsigned char byte = paction >> 3;
        BOOL status = ee_read((WORD)(EE_AREQ_STATUS+byte)) & (1<<bit);
        cbusMsg[d1] = nodeNumber >> 8;
        cbusMsg[d2] = nodeNumber & 0xFF;
        cbusMsg[d3] = eventNumber >> 8;
        cbusMsg[d4] = eventNumber & 0xFF;
        if (status) {
            cbusMsg[d0] = OPC_ARON;    
        } else {
            cbusMsg[d0] = OPC_AROF;
        }
        cbusSendMsg(ALL_CBUS, cbusMsg);
    }
}


/**
 * Remove event.
 * @param nodeNumber
 * @param eventNumber
 * @return error or 0 for success
 */
unsigned char removeEvent(WORD nodeNumber, WORD eventNumber) {
    EventTableFlags f;
    // need to delete this action from the Event table. 
    unsigned char tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex == NO_INDEX) return CMDERR_INV_EV_IDX; // not found
    // found the event to delete
    // set the free flag
    writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), 0xff);
    // Now follow the next pointer
    f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
    while (f.continued) {
        tableIndex = readFlashBlock((WORD)(& eventTable[tableIndex].next));
        // the continuation flag of this entry should be set but I'm 
        // not going to check as I wouldn't know what to do if it wasn't set
                    
        // set the free flag
        writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), 0xff);
        f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
    }
#ifdef HASH_TABLE
    // easier to rebuild from scratch
    rebuildHashtable();
#endif
    flushFlashImage();

    return 0;
}
/**
 * Add an event/EV.
 * Teach or reteach an event associated with an action. 
 * This may (optionally) need to create a new event and then optionally
 * create additional chained entries. All newly allocated table entries need
 * to be initialised.
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
                    writeFlashByte((BYTE*)&(eventTable[tableIndex].evs[e]), NO_ACTION);
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
        WORD nn, en;
        if (tableIndex == NO_INDEX) return NO_INDEX;
        nn = getNN(tableIndex);
        en = getEN(tableIndex);
        if ((nn == nodeNumber) && (en == eventNumber)) {
            return tableIndex;
        }
    }
#else
    for (unsigned char tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        EventTableFlags f;
        f.asByte = readFlashBlock((WORD)(& eventTable[i].flags));
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
 * Write an EV value to an event
 * @param tableIndex the index into the event table
 * @param evNum the EV number (0 for the produced action)
 * @param evVal
 * @return 0 if success otherwise the error
 */
unsigned char writeEv(unsigned char tableIndex, BYTE evNum, BYTE evVal) {
    EventTableFlags f;
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
        } else {
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
                        writeFlashByte((BYTE*)&(eventTable[nextIdx].evs[e]), NO_ACTION); // clear the EVs
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
    if (f.maxEvUsed < evNum) {
        f.maxEvUsed = evNum;
        writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), f.asByte);
    }
    return 0;
}
 
/**
 * Return an EV value for an event.
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
        f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
        evNum -= EVENT_TABLE_WIDTH;
    }
    if (evNum > f.maxEvUsed) {
        return -CMDERR_NO_EV;
    }
    // it is within this entry
    return readFlashBlock((WORD)(&(eventTable[tableIndex].evs[evNum])));
}

/**
 * Return all the EV values for an event. EVs are put into the global evs array.
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
                evs[evNum] = NO_ACTION;
            }
            return 0;
        }
        tableIndex = readFlashBlock((WORD)(&(eventTable[tableIndex].next)));
    }
    return 0;
}

/**
 * Return the NN for an event.
 * Getter so that the application code can obtain information about the event.
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
            EventTableFlags f;
            f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
            if ( ! f.freeEntry) {
                unsigned char e;
                for (e=0; e<EVENT_TABLE_WIDTH; e++) {
                    if (readFlashBlock((WORD)(&(eventTable[tableIndex].evs[e]))) == action) {
                        writeEv(tableIndex, e, NO_ACTION);
                    }
                }
            }
        }
        // now check to see if any events can be removed
        for (tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
            BOOL remove = TRUE;
            if ( validStart(tableIndex)) {
                unsigned char e;
                for (e=0; e<EVENT_TABLE_WIDTH; e++) {
                    if (readFlashBlock((WORD)(&(eventTable[tableIndex].evs[e]))) != NO_ACTION) {
                        remove = FALSE;
                        break;
                    }
                }
            }
            if (remove) {
                writeFlashByte((BYTE*)&(eventTable[tableIndex].flags.asByte), 0xFF);
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
 * @return TRUE if the produced event is found
 */ 
Event producedEvent;
BOOL getProducedEvent(PRODUCER_ACTION_T paction) {
    if ((paction < ACTION_PRODUCER_BASE) || (paction >= ACTION_PRODUCER_BASE + NUM_PRODUCER_ACTIONS)) return NULL;    // not a produced valid action
#ifdef HASH_TABLE
    if (action2Event[paction-ACTION_PRODUCER_BASE] == NO_INDEX) return FALSE;
    producedEvent.NN = getNN(action2Event[paction-ACTION_PRODUCER_BASE]);
    producedEvent.EN = getEN(action2Event[paction-ACTION_PRODUCER_BASE]);
    return TRUE;
#else
    for (unsigned char tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        if (validStart(tableIndex)) {
            if (readFlashBlock((WORD)(& eventTable[tableIndex].evs[e])) == paction) {
                ret.NN = getNN(action2Event[paction-ACTION_PRODUCER_BASE])
                ret.EN = getEN(action2Event[paction-ACTION_PRODUCER_BASE]);
                return &ret;
            }
        }
    }
    return NULL,
#endif
}

/**
 * Send a produced Event.
 * @param paction the produced action
 * @param on indicated whether an ON event or an OFF event should be sent
 * @return true if the event was successfully sent
 */
BOOL sendProducedEvent(PRODUCER_ACTION_T paction, BOOL on) {
    unsigned char bit = paction & 0x7;
    unsigned char byte = paction >> 3;
    unsigned char status = ee_read((WORD)(EE_AREQ_STATUS+byte));
    if (on) {
        status |= (1<<bit);
    } else {
        status &= ~(1<<bit);
    }
    ee_write((WORD)(EE_AREQ_STATUS+byte), status);
    
    if (getProducedEvent(paction)) {
        return cbusSendEvent( 0, producedEvent.NN, producedEvent.EN, on );
    }
    // Didn't find a provisioned event so now check for programmed default events
    // The default events are application specific so call back into application space
    if (getDefaultProducedEvent(paction)) {
        return cbusSendEvent( 0, producedEvent.NN, producedEvent.EN, on );
    }
    // Didn't find a provisioned event so instead send a debug message containing the action
    cbusMsg[d3] = paction & 0xFF;
    cbusMsg[d4] = paction >> 8;
    cbusMsg[d5] = on;
    cbusMsg[d6] = status;
    cbusMsg[d7] = 0;
    return cbusSendOpcNN(ALL_CBUS, OPC_ACDAT, -1, cbusMsg);
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
    int a;
#ifdef PRODUCED_EVENTS
    // first initialise to nothing
    PRODUCER_ACTION_T paction;
    for (paction=0; paction<NUM_PRODUCER_ACTIONS; paction++) {
        action2Event[paction] = NO_INDEX;
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
                paction = a;
                if ((paction >= ACTION_PRODUCER_BASE) && (paction-ACTION_PRODUCER_BASE< NUM_PRODUCER_ACTIONS)) {
                    action2Event[paction-ACTION_PRODUCER_BASE] = tableIndex;
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

