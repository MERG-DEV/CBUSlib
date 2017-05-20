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

#include "FLiM.h"
#include "callbacks.h"

// Local prototypes

void evRemove( BYTE evIndex );



#pragma code APP
#pragma udata

BYTE    prevIndex;   // Index of event entry that links to current one

// eventsInit called during initialisation - initialises event support 

void eventsInit(void)

{
    //TODO - Can we assume that the uninitialised event table will be set to all 0xFF, ie: all entries free?
    // If so, then no initialisation action required here, will need to test this

    prevIndex = 0;
} // eventsInit




// Clear all events
void doNnclr(void)
{
    BYTE    evIndex;

	if (flimState == fsFLiMLearn)
	{
            for (evIndex=0; evIndex < EVT_NUM; evIndex++)
                evRemove(evIndex);
            flushFlashImage();
     
            ee_write(EE_EV_COUNT, 0);           // Set event count to zero
            ee_write(EE_EV_FREE, EVT_NUM - 1);  // and free slots

            cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
        }
	else
	{
            cbusMsg[d3] = CMDERR_NOT_LRN;
            cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
 	}
} // doNnclr

// Read number of available event slots
void doNnevn(void)
{
    cbusMsg[d3] = ee_read(EE_EV_FREE);
    cbusSendOpcMyNN( 0, OPC_EVNLF, cbusMsg );
} // doNnevn

// Read all stored events
void doNerd(void)
{
	//TODO send response OPC_ENRSP (presumably once for each event and EV?  Study FCU interaction with existing modules)
} // doNerd

// Read number of stored events
void doRqevn(void)
{
    cbusMsg[d3] = ee_read(EE_EV_COUNT);
    cbusSendOpcMyNN( 0, OPC_NUMEV, cbusMsg );
} // doRqevn


// Unlearn event
void doEvuln(WORD nodeNumber, WORD eventNumber)
{
    // Find event in event table (may be more than one entry)
    // and remove
    BYTE    evIndex;
  
    if (evIndex = findEvent( nodeNumber, eventNumber, FALSE  ) != 0)
    {
        evRemove( evIndex );
        flushFlashImage();
        cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
    }

} // doEvuln


// Remove entry from event table (does not flush Flash buffer)

void evRemove( BYTE evIndex )

{
        BYTE            evNum;
        EventTableEntry eventCopy;

        // Unlink from event chain
        if (prevIndex != 0)
            setFlashByte(&(EVTPtr[prevIndex].event.nextEvent), EVTPtr[evIndex].event.nextEvent);

        eventCopy.event.evtFlags.free = 1;                        // Mark entry in use
        eventCopy.event.node_id = 0;                              // Set node id for event
        eventCopy.event.evt_id  = 0;                             // Set event or device number

        for (evNum = 0; evNum < EVperEVT; evNum++)
            eventCopy.ev[evNum] = 0xFF;                          // Set event variable value

        setFlashBuffer(&(EVTPtr[evIndex]),&eventCopy,sizeof( EventTableEntry));
}

// Read an event variable by index
void doReval(void)
{
	// Get event index and event variable number from message
	// Send response with EV value
        //TODO - can the index be the hash table index, which would mean that not all indices in sequqnce are used?
        //TODO Is this used by FCU at present? If so what does it expect?
        //TODO Study FCU interaction with existing modules
} // doReval


// Read an event variable by event id
void doReqev(WORD nodeNumber, WORD eventNumber, BYTE evNum)
{
    BYTE evIndex;

        // Get event by event id and event variable number from message
	// Send EVANS response with EV value,

    if (evIndex = findEvent( nodeNumber, eventNumber, FALSE  ) != 0)
    {
        cbusMsg[d3] = eventNumber >> 8;
        cbusMsg[d4] = eventNumber & 0x00FF;
        cbusMsg[d5] = evNum;
        cbusMsg[d6] = EVTPtr[evIndex].ev[evNum];

        cbusSendOpcNN(0 , OPC_EVANS, nodeNumber, cbusMsg);
    }
    else
    {
        cbusMsg[d3] = CMDERR_INVALID_EVENT;
        cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
    }
} // doReqev


// Teach event whilst in learn mode
void doEvlrn(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal )
{
    BYTE            evIndex;
    EventTableEntry eventCopy;
   

    if (evIndex = findEvent( nodeNumber, eventNumber, TRUE  ) != 0)
    {
        // Cannot write directly to this structure as it is stored
        // in flash, so take a copy, modify it, then write back to flash

        // For a structure with a number of bytes, this is more efficient than writing each byte to flash individually

        memcpy( &eventCopy, &(EVTPtr[evIndex]), sizeof( EventTableEntry));

        eventCopy.event.evtFlags.free = 0;                        // Mark entry in use
        eventCopy.event.node_id = nodeNumber;                     // Set node id for event
        eventCopy.event.evt_id  = eventNumber;                    // Set event or device number
        eventCopy.ev[evNum] = evVal;                             // Set event variable value

        setFlashBuffer(&(EVTPtr[evIndex]),&eventCopy,sizeof( EventTableEntry));
        flushFlashImage();
        cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
    }
} // doEvlrn


// Teach event whilst in learn mode by index
// void doEvlrni(WORD nodeNumber, WORD eventNumber, BYTE evNum, BYTE evVal)
//{
	// Need to be in learn mode - index cannot be hash table entry, because it might not hash to that index
        // This implementation just hahses the node number and event number and  ignores the index - so the learn event with and without index are the same
        // So doEvlrn is called for both EVLRN AND EVLRNI opcodes and this routine is left commented out for the time being.
        // TODO Study FCU interaction with existing modules and see if this opcode used

//} // doEvlrni



// Search the event table for the specified event, return the index,
// optionally create new entry, return zero if not found or if table full when trying to create
// Sets prevIndex to index of event that links to returned one

BYTE findEvent( WORD eventNode, WORD eventNum, BOOL createEntry  )
{
    BYTE    eventIndex;
    BYTE    searchCount;
    BOOL    notFound;

    prevIndex = 0;
    eventIndex = eventHash(eventNode & 0xFF, eventNum & 0xFF );
    
    while ((!EVTPtr[eventIndex].event.evtFlags.freeEntry) && (notFound = (EVTPtr[eventIndex].event.node_id != eventNode) || (EVTPtr[eventIndex].event.evt_id != eventNum)))
    {
        if ((EVTPtr[eventIndex].event.nextEvent) != 0)
        {
            prevIndex = eventIndex;
            eventIndex = EVTPtr[eventIndex].event.nextEvent;
        }
    }    

    if (notFound && createEntry)
    {
        searchCount = 0;
        prevIndex = eventIndex;

        while (!EVTPtr[++eventIndex].event.evtFlags.freeEntry && (searchCount++ < 0xFF ));

        if (EVTPtr[eventIndex].event.evtFlags.free) // If we can't find a free entry, then the table is full
        {
            if ((prevIndex != 0) && (prevIndex != eventIndex))
            {
                // EVTPtr[prevIndex].event.nextEvent = eventIndex;                  // Link from previous event on same hash
                setFlashByte( &(EVTPtr[prevIndex].event.nextEvent), eventIndex );   // write to flash buffer
            }
            // EVTPtr[eventIndex].event.evtFlags.freeEntry = FALSE;                 // No longer a free entry, but now set in caller for flash write efficiency
            notFound = FALSE;
        }    
    }
    
    if (notFound)
        eventIndex = 0;     // Return 0 if not found, or if could not create due to table full
    
    return (eventIndex);    // not found
   
} // findEvent



// The hash table uses an algorithm on the least significant byte of each of the node number and event number.

// If we used just the node number, then all short events would hash to the same number
// If we used just the event number, then for long events the vast majority would be 1 to 8, so not giving a very good spread.

// The majority of long events will be between 1 and 16, as most modules default to event numbers starting from 1. 
// To give a good spread of hash values from 1 to 255, the event number is left shifted 3 bits


// This algorithm combines the LS 7 bits of the node number with bits 0-3 of the event number, hopefully producing a good spread for layouts with either
// predominantly short events, long events or a mixture

//  This also means that for layouts using the default FLiM node numbers from 256, we are effectively starting from zero as far as the hash algorithm is concerned.

// Table entry 0 is not used so return 1 if the hash gives a result of 0.


// Calculate hash value, this is the index into the hash table

BYTE eventHash( BYTE nodeByte, BYTE eventBYTE )
{
    BYTE hashResult;

    hashResult = ( nodeByte & 0x7F ) + ((eventBYTE << 3) & 0x7F );
    
    return( (hashResult == 0) ? ++hashResult : hashResult );
   
}

BOOL parseCbusEvent( BYTE *msg )

{
    overlay BOOL    cmdProcessed = TRUE;
    BYTE            eventIndex;
    BOOL            foundEvent;


    if (foundEvent = ((msg[d0] & EVENT_SET_MASK) == EVENT_SET_MASK) && ((msg[d0] & EVENT_CLR_MASK == 0) ))  // Check correct bits in opcode set or clear for an event
    {
        if ((eventIndex = findEvent(msg[d1]<<8+msg[d2],msg[d3]<<8+msg[d4], FALSE)) != 0)
        {
            processEvent( eventIndex, msg );
        }
    }
    return( foundEvent);
        
//    switch(msg[d0])
//    {
//        case OPC_ACON:     //
//            if (thisNN(msg))
//                break;  // Ignore if for us (already in learn) but drop through to exit learn mode if not addressed to us
//
//        case OPC_NNULN:
//            // Release node from learn mode
//             flimState = fsFLiM;
//            break;
//    }
} 

// Where an event entry spans more than one event table entry, find the next continuation 
// Returns index to the new entry if there is a continuation,. or 0 if not

BYTE findEventContinuation(BYTE eventIndex)
        
{
    WORD    eventNode, eventID;
    
    if (EVTPtr[eventIndex].event.evtFlags.conctinues)
    {
        eventNode = EVTPtr[eventIndex].event.node_id;
        eventID = EVTPtr[eventIndex].event.evt_id;
        
        eventIndex = EVTPtr[eventIndex].event.nextEvent;
        
        while ((eventIndex != 0) && ((EVTPtr[eventIndex].event.node_id != eventNode) || (EVTPtr[eventIndex].event.evt_id != eventID)))
           eventIndex = EVTPtr[eventIndex].event.nextEvent;
    }    
    else 
        eventIndex = 0;
    
    return eventIndex;
}        





