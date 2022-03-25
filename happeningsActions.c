/*

 Copyright (C) Ian Hogg

 Happenings and Actions Routines for CBUS event management - part of CBUS libraries for PIC 18F

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
        05/06/21     Ian Hogg        - split from events.c
 
 * Currently written for:
 *  * C18 compiler
 * 
 * This file used the following PIC peripherals:
 *  * none
 * 
 */
#include <stddef.h>
#include "module.h"
#include "events.h"
#include "romops.h"
#include "happeningsActions.h"
#include "cbus.h"


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
BOOL getProducedEvent(HAPPENING_T happening) {
#ifndef HASH_TABLE
    unsigned char tableIndex;
#endif
    if ((happening < HAPPENING_BASE) || (happening >= HAPPENING_BASE + NUM_HAPPENINGS)) return FALSE;    // not a produced valid action
#ifdef HASH_TABLE
    if (happening2Event[happening-HAPPENING_BASE] == NO_INDEX) return FALSE;
    producedEvent.NN = getNN(happening2Event[happening-HAPPENING_BASE]);
    producedEvent.EN = getEN(happening2Event[happening-HAPPENING_BASE]);
    return TRUE;
#else
    for (tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        if (validStart(tableIndex)) {
            if (readFlashBlock((WORD)(& eventTable[tableIndex].evs[0])) == happening) {
                producedEvent.NN = getNN(tableIndex);
                producedEvent.EN = getEN(tableIndex);
                return TRUE;
            }
        }
    }
    return FALSE;
#endif
}


/**
 * Send a produced Event.
 * @param happening the produced action
 * @param on indicated whether an ON event or an OFF event should be sent
 * @return true if the event was successfully sent
 */
BOOL sendProducedEvent(HAPPENING_T happening, BOOL on) {
#ifdef AREQ_SUPPORT
    unsigned char thisBit = (unsigned char)(happening & 0x7);
    unsigned char thisByte = (unsigned char)(happening >> 3);
    unsigned char status = ee_read((WORD)(EE_AREQ_STATUS+thisByte));
    if (on) {
        status |= (1<<thisBit);
    } else {
        status &= ~(1<<thisBit);
    }
    ee_write((WORD)(EE_AREQ_STATUS+thisByte), status);
#endif
    
    if (getProducedEvent(happening)) {
        return cbusSendEvent( 0, producedEvent.NN, producedEvent.EN, on );
    }
    // Didn't find a provisioned event so now check for programmed default events
    // The default events are application specific so call back into application space
    if (getDefaultProducedEvent(happening)) {
        if (producedEvent.EN != 0)
            return cbusSendEvent( 0, producedEvent.NN, producedEvent.EN, on );
        // lie and say we sent it. Ian changed in 2K as fix for SoD
        return TRUE;
    }
#ifdef DEBUG_PRODUCED_EVENTS
    // Didn't find a provisioned event so instead send a debug message containing the action
    cbusMsg[d3] = happening & 0xFF;
    cbusMsg[d4] = happening >> 8;
    cbusMsg[d5] = on;
    cbusMsg[d6] = status;
    cbusMsg[d7] = 0;
    return cbusSendOpcNN(ALL_CBUS, OPC_ACDAT, -1, cbusMsg);
#else
    // Didn't find an event to send so lie and say we sent it otherwise the
    // caller would retry in an infinite loop.
    return TRUE;
#endif
}

/**
 * Delete all occurrences of the Happening.
 * @param action
 */
void deleteHappeningRange(HAPPENING_T action, unsigned char number) {
    unsigned char tableIndex;
    for (tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        if ( validStart(tableIndex)) {
            EventTableFlags f;
            unsigned char pa;
            f.asByte = readFlashBlock((WORD)(&(eventTable[tableIndex].flags.asByte)));
            pa = readFlashBlock((WORD)(&(eventTable[tableIndex].evs[0])));
            if ((pa >= action) && (pa < action+number)) {
                writeEv(tableIndex, 0, EV_FILL);
                checkRemoveTableEntry(tableIndex);
            }                
        }
    }
    flushFlashImage();
#ifdef HASH_TABLE
    rebuildHashtable();                
#endif
}


#ifdef AREQ_SUPPORT

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
void doAreq(WORD nodeNumber, WORD eventNumber) {
    HAPPENING_T happening;
    int ev0;
    
    unsigned char tableIndex = findEvent(nodeNumber, eventNumber);
    if (tableIndex == NO_INDEX) return;
    // found the matching event
    ev0 = getEv(tableIndex, 0); // get the Happening
    if (ev0 < 0) {
        doError((unsigned char)(-ev0));
        return;
    }
    happening = (unsigned char)ev0;
    if ((happening >= HAPPENING_BASE) && (happening-HAPPENING_BASE< NUM_HAPPENINGS)) {
        unsigned char thisBit = (unsigned char)(happening & 0x7);
        unsigned char thisByte = (unsigned char)(happening >> 3);
        BOOL status = ee_read((WORD)(EE_AREQ_STATUS+thisByte)) & (unsigned char)(1<<thisBit);
        if (nodeNumber == 0) {
            cbusMsg[d1] = nodeID >> 8;
            cbusMsg[d2] = nodeID & 0xFF;
        } else {
            cbusMsg[d1] = nodeNumber >> 8;
            cbusMsg[d2] = nodeNumber & 0xFF;
        }
        cbusMsg[d3] = eventNumber >> 8;
        cbusMsg[d4] = eventNumber & 0xFF;
        if (status) {
            cbusMsg[d0] = (unsigned char)(nodeNumber == 0 ? OPC_ARSON : OPC_ARON);    
        } else {
            cbusMsg[d0] = (unsigned char)(nodeNumber == 0 ? OPC_ARSOF : OPC_AROF);
        }
        cbusSendMsg(ALL_CBUS, cbusMsg);
    }
}
#endif /* AREQ_SUPPORT */
#endif /* PRODUCE_EVENTS */

/**
 * Delete all occurrences of the consumer action.
 * @param action
 */
void deleteActionRange(ACTION_T action, unsigned char number) {
    unsigned char tableIndex;
    for (tableIndex=0; tableIndex < NUM_EVENTS; tableIndex++) {
        if (validStart(tableIndex)) {
            BOOL updated = FALSE;
            unsigned char e;
            if (getEVs(tableIndex)) {
                return;
            }
                
            for (e=1; e<EVperEVT; e++) {
                if ((evs[e] >= action) && (evs[e] < action+number)) {
                    writeEv(tableIndex, e, EV_FILL);
                    updated = TRUE;
                }
            }
            if (updated) {
                checkRemoveTableEntry(tableIndex);
            }
        }
    }
    flushFlashImage();
#ifdef HASH_TABLE
    rebuildHashtable();                
#endif
}



