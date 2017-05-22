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

 */
#include "events.h"
#include "../CANMIOfirmware/module.h"
#include "FLiM.h"
#include <stddef.h>

// forward references
void clearEvent2Action(void);
void rebuildHashtable(void);
unsigned char getHash(WORD nn, WORD en);

extern void processEvent(unsigned char action, BYTE * msg);

//Events are stored in Flash just below NVs
/*
 * The Action to Event table.
 */
const Event action2Event[NUM_PRODUCER_ACTIONS] @AT_ACTION2EVENT = {0}; // should initialise entire array to all zeros


/*
 * The Event to Action storage.
 */
typedef struct {
    Event event;
    BYTE actions[EVperEVT];
} Event2Action;
const Event2Action event2Action[NUM_CONSUMED_EVENTS] @AT_EVENT2ACTION;

// The hashtable to find the Event within the event2Action table. Stored in RAM
BYTE eventChains[HASH_LENGTH][CHAIN_LENGTH];

/**
 * eventsInit called during initialisation - initialises event support.
 * Called after power up to initialise RAM.
 */
void eventsInit( void ) {
    rebuildHashtable();
} //eventsInit

/**
 * Clear all Events.
 */
void doNnclr(void) {
    if (flimState == fsFLiMLearn) {
        clearAction2Event();
        clearEvent2Action();
        rebuildHashtable();
    } else {
            cbusMsg[d3] = CMDERR_NOT_LRN;
            cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
 	}
} //doNnclr

/**
 * Read number of available event slots.
 * This returned the number of unused slots in the Consumed event Event2Action table.
 */
void doNnevn(void)
{
    //Pete's original code kept a counter in EEPROM but here I count the number
    // of unused slots.
    unsigned char count = 0;
    for (unsigned char i=0; i<NUM_CONSUMED_EVENTS; i++) {
        if ((event2Action[i].event.NN == NO_EVENT) && (event2Action[i].event.EN == NO_EVENT)) {
            count++;
        }
    }
    cbusMsg[d3] = count;
    cbusSendOpcMyNN( 0, OPC_EVNLF, cbusMsg );
} // doNnevn

// Read all stored events
void doNerd(void)
{
	//TODO send response OPC_ENRSP (presumably once for each event and EV?  Study FCU interaction with existing modules)
    // The CBUS spec doesn't seem to cover what do with the produced events - should we return both and fake an Index?
    unsigned char i;
    for (i=0; i<NUM_PRODUCER_ACTIONS; i++) {
        if ((action2Event[i].NN != NO_EVENT) || (action2Event[i].EN != NO_EVENT)) {
            cbusMsg[d3] = action2Event[i].NN>>8;
            cbusMsg[d4] = action2Event[i].NN&0xff;
            cbusMsg[d5] = action2Event[i].EN>>8;
            cbusMsg[d6] = action2Event[i].EN&0xff;
            cbusMsg[d7] = i;
            cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg );
        }
    }
    for (i=0; i<NUM_CONSUMED_EVENTS; i++) {
        if ((event2Action[i].event.NN != NO_EVENT) || (event2Action[i].event.EN != NO_EVENT)) {
            //for (unsigned char a=0; a<EVperEvt; a++) {  
                cbusMsg[d3] = event2Action[i].event.NN>>8;
                cbusMsg[d4] = event2Action[i].event.NN&0xff;
                cbusMsg[d5] = event2Action[i].event.EN>>8;
                cbusMsg[d6] = event2Action[i].event.EN&0xff;
                cbusMsg[d7] = i;
                cbusSendOpcMyNN( 0, OPC_ENRSP, cbusMsg );
            //}
        }
    }
} // doNerd

/**
 * Read number of stored events
 * This returned the number of unused slots in the Consumed event Event2Action table.
 */
void doRqevn(void)
{
    //Pete's original code kept a counter in EEPROM but here I count the number
    // of unused slots.
    unsigned char count = 0;
    for (unsigned char i=0; i<NUM_CONSUMED_EVENTS; i++) {
        if ((event2Action[i].event.NN != NO_EVENT) || (event2Action[i].event.EN != NO_EVENT)) {
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
void doEvuln(WORD nodeNumber, WORD eventNumber)
{
    // need to delete this action from the Produced and Consumed tables
    // delete from produced first
    unsigned char a;
    for (a=0; a<NUM_ACTIONS; a++) {
        if ((eventNumber == action2Event[a].EN) && (nodeNumber == action2Event[a].NN)) {
            writeFlashImage((BYTE*)&(action2Event[a].NN), NO_EVENT);
            writeFlashImage((BYTE*)&(action2Event[a].NN)+1, NO_EVENT);
            writeFlashImage((BYTE*)&(action2Event[a].EN), NO_EVENT);
            writeFlashImage((BYTE*)&(action2Event[a].EN)+1, NO_EVENT);
        }
    }
    
    // now delete from consumed
    unsigned char evtIdx;
    for (evtIdx=0; evtIdx<NUM_CONSUMED_EVENTS; evtIdx++) {
        if ((eventNumber == event2Action[evtIdx].event.EN) && (nodeNumber == event2Action[evtIdx].event.NN)) {
            writeFlashImage((BYTE*)&(event2Action[evtIdx].event.NN), NO_EVENT);
            writeFlashImage((BYTE*)&(event2Action[evtIdx].event.NN)+1, NO_EVENT);
            writeFlashImage((BYTE*)&(event2Action[evtIdx].event.EN), NO_EVENT);
            writeFlashImage((BYTE*)&(event2Action[evtIdx].event.EN)+1, NO_EVENT);
            for (a=0; a<EVperEVT; a++) {
                writeFlashImage((BYTE*)&(event2Action[evtIdx].actions[a]), NO_ACTION);
            }
            // easier to rebuild from scratch
            rebuildHashtable();
            return;
        }
    }
    flushFlashImage();
}

/**
 *  Read an event variable by index.
 * Again not clear what the CBUS spec actually needs here as the Index is implementation
 * specific and doesn't really apply in my implementation using 2 tables.
 */
void doReval(void)
{
	// Get event index and event variable number from message
	// Send response with EV value
        //TODO - can the index be the hash table index, which would mean that not all indices in sequence are used?
        //TODO Is this used by FCU at present? If so what does it expect?
        //TODO Study FCU interaction with existing modules
    unsigned char idx = cbusMsg[d3];
    unsigned char action = cbusMsg[d4];
    if (idx < NUM_CONSUMED_EVENTS) {
        cbusMsg[5] = event2Action[idx].actions[action];
        cbusSendOpcMyNN( 0, OPC_NEVAL, cbusMsg );
    } else {
        cbusMsg[d3] = CMDERR_INVALID_EVENT;
        cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
    }
} // doReval


/**
 * Read an event variable by event id.
 * TODO Again not quite sure what to do here as there can be multiple EVs as a result.
 * TODO Also this only checks the Consumed events and not the Produced events.
 * @param nodeNumber
 * @param eventNumber
 * @param evNum
 */
void doReqev(WORD nodeNumber, WORD eventNumber, BYTE evNum)
{
    // get the event
    unsigned char hash = getHash(nodeNumber, eventNumber);
    unsigned char chainIdx;
 
    for (chainIdx=0; chainIdx<CHAIN_LENGTH; chainIdx++) {
        unsigned char evtIdx = eventChains[hash][chainIdx];
        if (evtIdx == NO_INDEX) return;    // no more left to check - no action
        // need to check in case of hash collision
        const Event * otherEvent = (const Event*)&(event2Action[evtIdx].event);
        if ((eventNumber == otherEvent->EN) && (nodeNumber == otherEvent->NN)) {
            // found the correct consumed event - now get the action
            cbusMsg[d3] = eventNumber >> 8;
            cbusMsg[d4] = eventNumber & 0x00FF;
            cbusMsg[d5] = evNum;
            cbusMsg[d6] = event2Action[evtIdx].actions[evNum];
            cbusSendOpcMyNN( 0, OPC_EVANS, cbusMsg);
            return;
        }
    }
    cbusMsg[d3] = CMDERR_INVALID_EVENT;
    cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
}

/**
 * Teach event whilst in learn mode.
 * Teach or reteach an event associated with an action. This updates either the 
 * Action2Event (Produced events) table or the Event2Action (Consumed events) table.
 * If this is a Consumed event then also update the hash table.
 * @param nodeNumber
 * @param eventNumber
 * @param evNum not used
 * @param evVal the action
 * @return 
 */
void doEvlrn(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal ) {
    if (evVal >= NUM_ACTIONS) {
        cbusMsg[d3] = CMDERR_INV_EV_IDX;
        cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
        return;
    }
    if (evVal < NUM_PRODUCER_ACTIONS) {
        // teach a PRODUCED action
        // Just write the action to the action2Event table
        writeFlashImage((BYTE*)&action2Event[evVal].NN, nodeNumber & 0xff);
        writeFlashImage((BYTE*)&action2Event[evVal].NN+1, nodeNumber >> 8);
        writeFlashImage((BYTE*)&action2Event[evVal].EN, eventNumber & 0xff);
        writeFlashImage((BYTE*)&action2Event[evVal].EN+1, eventNumber >> 8);
        flushFlashImage();
        cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
        return;
    } else {
        // teach a CONSUMED action
        // check it we already have this event
        unsigned char hash = getHash(nodeNumber, eventNumber);
        unsigned char chainIdx;
        for (chainIdx=0; chainIdx<CHAIN_LENGTH; chainIdx++) {
            unsigned char evtIdx = eventChains[hash][chainIdx];
            if (evtIdx == NO_INDEX) {
                // it isn't in the table
                // find a slot in the table
                for (evtIdx=0; evtIdx<NUM_CONSUMED_EVENTS; evtIdx++) {
                    if ((event2Action[evtIdx].event.NN == NO_EVENT) && (event2Action[evtIdx].event.EN == NO_EVENT)) {
                        // found a spare slot
                        eventChains[hash][chainIdx] = evtIdx;
                        writeFlashImage((BYTE*)&(event2Action[evtIdx].event.NN), nodeNumber & 0xff);
                        writeFlashImage((BYTE*)&(event2Action[evtIdx].event.NN)+1, nodeNumber >> 8);
                        writeFlashImage((BYTE*)&(event2Action[evtIdx].event.EN), eventNumber & 0xff);
                        writeFlashImage((BYTE*)&(event2Action[evtIdx].event.EN)+1, eventNumber >> 8);
                        writeFlashImage((BYTE*)&(event2Action[evtIdx].actions[0]), evVal);
                        flushFlashImage();
                        cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
                        return;
                    } 
                }
                // no slots available
                cbusSendOpcMyNN( 0, CMDERR_TOO_MANY_EVENTS, cbusMsg);
                return;
            }
            // need to check in case of hash collision
            const Event * otherEvent = (const Event*)&(event2Action[evtIdx].event);
            if ((eventNumber == otherEvent->EN) && (nodeNumber == otherEvent->NN)) {
                // found the correct consumed event - now add this action to the list
                // look for a spare action space
                unsigned char a;
                for (a=0; a<EVperEVT; a++) {
                    if (event2Action[evtIdx].actions[a] == evVal) {
                        // already there
                        //WRACK or CmdErr?
                        cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
                        return;
                    }
                    if (event2Action[evtIdx].actions[a] == NO_ACTION) {
                        writeFlashByte((BYTE*)&(event2Action[evtIdx].actions[a]), evVal);
                        cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
                        return;
                    }
                }
                // ERROR = NO EVs left
                cbusSendOpcMyNN( 0, CMDERR_INV_EV_IDX, cbusMsg);
                return;
            }
        }
    }
}


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
unsigned */
 char getHash(WORD nn, WORD en) {
    unsigned char hash;
    // need to hash the NN and EN to a uniform distribution across HASH_LENGTH
    hash = nn ^ (nn >> 8);
    hash = 7*hash + (en ^ (en>>8)); 
    // ensure it is within bounds of eventChains
    hash %= HASH_LENGTH;
    return hash;
}


/**
 * This Consumes a CBUS event if it has been provisioned.
 * @param msg
 * @return 
 */
BOOL parseCbusEvent(BYTE * msg) {
    Event evt;
    evt.NN = (msg[d1] << 8) + msg[d2];
    evt.EN = (msg[d3] << 8) + msg[d4];
    return doActions(&evt, msg);
} 


/**
 * Clear all the actions' events.
 */
void clearAction2Event(void) {
    unsigned char action;
    for (action=0; action<NUM_ACTIONS; action++) {
        writeFlashImage((BYTE*)&action2Event[action].NN, NO_INDEX);
        writeFlashImage((BYTE*)&action2Event[action].NN+1, NO_INDEX);
        writeFlashImage((BYTE*)&action2Event[action].EN, NO_INDEX);
        writeFlashImage((BYTE*)&action2Event[action].EN+1, NO_INDEX);
    }
    flushFlashImage();
}

void clearEvent2Action(void) {
    unsigned char idx;
    for (idx=0; idx<NUM_CONSUMED_EVENTS; idx++) {
        unsigned char j;
        for (j=0; j<sizeof(Event2Action); j++) {
            writeFlashByte((BYTE*)&event2Action[idx]+j, NO_ACTION);
        }
    }
}
/**
 * Initialise the RAM hash chain for reverse lookup of event to action. Uses the
 * data from the Flash Event2Action table.
 */
void rebuildHashtable(void) {
    // invalidate the current hash table
    unsigned char hash;
    unsigned char idx;
    clearChainTable();
    // now scan the event2Action table and populate the hash
    for (idx=0; idx<NUM_CONSUMED_EVENTS; idx++) {
        hash = getHash(event2Action[idx].event.NN, event2Action[idx].event.EN);
        unsigned char chainIdx;
        for (chainIdx=0; chainIdx<CHAIN_LENGTH; chainIdx++) {
            if (eventChains[hash][chainIdx] == NO_INDEX) {
                // available
                eventChains[hash][chainIdx] = idx;
                break;
            }
        }
    }
}

/**
 * Clear the RAM hash chain .
 */
void clearChainTable(void) {
    unsigned char h, chainIndex;
    // First fill the entire chains with NO_INDEX
    for (h=0; h<HASH_LENGTH; h++) {
        for (chainIndex=0; chainIndex<CHAIN_LENGTH; chainIndex++) {
            eventChains[h][chainIndex] = NO_INDEX;
        }
    }
}


/**
 * Get the Produced Event to transmit for the specified action.
 * @param action
 * @return the produced event or NULL if none has been provisioned
 */ 
const Event * getProducedEvent(unsigned char action) {
    if (action >= NUM_PRODUCER_ACTIONS)return NULL;    // not a produced valid action
    const Event * ep = &action2Event[action];
    if ((ep->EN == 0) && (ep->NN == 0)) return NULL;    // not provisioned
    return ep; 
}


/**
 * Perform the actions associated with this consumed event. 
 * Passes control back to the application to actually process the event.
 * @param e
 * @return true if the action was processed
 */
BOOL doActions(const Event * e, BYTE* msg) {
    unsigned char hash = getHash(e->NN, e->EN);
    unsigned char chainIdx;
    BOOL processed = FALSE;
    for (chainIdx=0; chainIdx<CHAIN_LENGTH; chainIdx++) {
        unsigned char evtIdx = eventChains[hash][chainIdx];
        if (evtIdx == NO_INDEX) return processed;    // no more left to check - no action
        // need to check in case of hash collision
        const Event * otherEvent = (const Event*)&(event2Action[evtIdx].event);
        if ((e->EN == otherEvent->EN) && (e->NN == otherEvent->NN)) {
            // found the correct consumed event - now process the actions
            unsigned char action;
            for (action=0; action<EVperEVT; action++) {
                if (action == NO_ACTION) return processed;    // done all the actions
                processEvent(action, msg);      // found it
                processed = TRUE;
            }
        }
    }
    return processed;
}



/**
 * Delete an action.
 * @param action
 */
void deleteAction(unsigned char action) {
    // need to delete this action from the Produced and Consumed tables
    // delete from produced first
    writeFlashImage((BYTE*)&(action2Event[action].NN), NO_EVENT);
    writeFlashImage((BYTE*)&(action2Event[action].NN)+1, NO_EVENT);
    writeFlashImage((BYTE*)&(action2Event[action].EN), NO_EVENT);
    writeFlashImage((BYTE*)&(action2Event[action].EN)+1, NO_EVENT);
    
    // now delete from consumed
    unsigned char evtIdx;
    for (evtIdx=0; evtIdx<NUM_CONSUMED_EVENTS; evtIdx++) {
        unsigned char a;
        for (a=0; a<EVperEVT; a++) {
            if (event2Action[evtIdx].actions[a] == action) {
                // shift the actions along - could be made more efficient by checking if we have reached a NO_ACTIO
                for (unsigned char aa = a; aa <EVperEVT-1; aa++) {
                    writeFlashImage((BYTE*)&(event2Action[evtIdx].actions[aa]), event2Action[evtIdx].actions[aa+1]);
                }
                writeFlashImage((BYTE*)&(event2Action[evtIdx].actions[EVperEVT-1]), NO_ACTION);
            }
        }
        // if the first (and hence all actions) is NO_ACTION then delete the entry entirely
        if (event2Action[evtIdx].actions[0] == NO_ACTION) {
            setFlashWord((WORD*)&event2Action[evtIdx].event.NN, NO_EVENT);
            setFlashWord((WORD*)&event2Action[evtIdx].event.EN, NO_EVENT);
        }
    }
    flushFlashImage();
    // easier to rebuild from scratch
    rebuildHashtable();
}





