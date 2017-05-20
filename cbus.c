/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 CBUS transmit and receive routines - part of CBUS libraries for PIC 18F
 This source module handles the interface to different transport routines
 The interface to this module from the caller is transport independent

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

*/

#include "can18.h"


#include <GenericTypeDefs.h>
#include "cbus.h"
#include <romops.h>
#include <EEPROM.h>

WORD    nodeID;
BYTE    cbusMsg[pktsize]; // Global buffer for fast access to CBUS packets - do NOT use in ISRs as would not be re-entrant

#pragma code APP


// Initialise CBUS  - Note: CANID only used when bus type is CAN

void cbusInit( WORD initNodeID  )
{
    initTicker();  // Background ticker used for time delays

    nodeID = ee_read_short( EE_NODE_ID );

    if (nodeID == 0xFFFF)
        nodeID = initNodeID; // Use default if uninitialised

    #if defined(CBUS_OVER_CAN)
        canInit(0,0);  // use default canid
    #endif

    #if defined(CBUS_OVER_MIWI)
        //TODO Move MiWi initialisation here
    #endif

    #if defined(CBUS_OVER_TCP)
        //TODO Look at moving Ethernet MLA library init toh here for use with CANEther
    #endif


}


// Check for CBUS message received, return TRUE with message in msg buffer if so
BOOL cbusMsgReceived( BYTE cbusNum, BYTE *msg )
{

#if defined(CBUS_OVER_CAN)
    // No other processing at this level at the moment
    if (cbusNum == CBUS_OVER_CAN)
    {
        return( canbusRecv( (CanPacket *) msg ));
    }
#endif

#if defined(CBUS_OVER_MIWI)
    if (cbusNum == CBUS_OVER_MIWI)
    {
        // Put Miwi receive stuff in here
    }
#endif

}

/*
 * Send single byte frame - opcode only
 */
void cbusSendSingleOpc(BYTE cbusNum, BYTE opc )
{
    cbusMsg[d0] = opc;
    cbusSendMsg(cbusNum, cbusMsg);
}



/*
 * Send opcode plus our node number, can be usd to send simple 3-byte frame, or may have further bytes prepared in the buffer by the caller
 */
void cbusSendOpcMyNN(BYTE cbusNum, BYTE opc, BYTE *msg)
{
    msg[d0] = opc;
    cbusSendMsgMyNN(cbusNum, msg);
}


/*
 * Send opcode plus specified node number, can be usd to send simple 3-byte frame, or may have further bytes prepared in the buffer by the caller
 */
void cbusSendOpcNN(BYTE cbusNum, BYTE opc, WORD nodeID, BYTE *msg)
{
	msg[d0] = opc;
	cbusSendMsgNN(cbusNum, nodeID, msg);
}


// Send long CBUS event, no data, using our node number

void cbusSendMyEvent( BYTE cbusNum, WORD eventNum, BOOL onEvent )
{
    BYTE    msg[d0+4];

    cbusSendEventWithData( cbusNum, -1, eventNum, onEvent, msg, 0);
}

// Send CBUS event, no data, using specified node number.
// Send short event if eventNode is 0


void cbusSendEvent( BYTE cbusNum, WORD eventNode, WORD eventNum, BOOL onEvent )
{
    BYTE    msg[d0+4];

    cbusSendEventWithData( cbusNum, eventNode, eventNum, onEvent, msg, 0);
}



// Send CBUS event with optional data bytes - works out appropriate opcode
void cbusSendEventWithData( BYTE cbusNum, WORD eventNode, WORD eventNum, BOOL onEvent, BYTE *msg, BYTE datalen )
{
    msg[d0] = OPC_ACON;         // Start with long event opcode

    if (eventNode == 0)
    {
        msg[d0] |= 0x08;        // Short event opcode
        eventNode = nodeID;     // Add module node id for diagnostics
    }

    if (!onEvent)
        msg[d0] |= 0x01;        // Off event

    if (datalen > 0)
        msg[d0] |= (datalen << 5);   // Opcode for event + data

    msg[d3] = eventNum>>8;
    msg[d4] = eventNum & 0xFF;

    cbusSendMsgNN( cbusNum, eventNode, msg );

    #if defined(CBUS_OVER_CAN)
        if ((cbusNum == CBUS_OVER_CAN) || (cbusNum == 0xFF) )
            canQueueRx( msg );      // Queue event into receive buffer so module can be taugjt its own events

    #endif

} // cbusSendEventWithData

/*
 * Send a CBUS message, putting our Node Number in first two data bytes
 */
void cbusSendMsgMyNN(BYTE cbusNum, BYTE *msg)
{
    msg[d1] = nodeID>>8;
    msg[d2] = nodeID & 0xFF;
    cbusSendMsg(cbusNum, msg);
}

/*
 * Send a CBUS message, work out node Number and put it in first two data bytes
 */
void cbusSendMsgNN(BYTE cbusNum, WORD eventNode, BYTE *msg)
{
    if (eventNode == -1)
        eventNode = nodeID; // Use node id for this module

    msg[d1] = eventNode>>8;
    msg[d2] = eventNode & 0xFF;
    cbusSendMsg(cbusNum, msg);
}


/*
 * Send a CBUS message where data bytes have already been loaded
 * Works out data length from opcode
 * If cbusNum is set to 0xFF, transmits on all defined CBUS connections
 */
void cbusSendMsg(BYTE cbusNum, BYTE *msg)
{
    #if defined(CBUS_OVER_CAN)
        if ((cbusNum == CBUS_OVER_CAN) || (cbusNum == 0xFF) )
            canSend( msg, (msg[d0] >> 5)+1);	// data length from opcode

    #endif


    #if defined(CBUS_OVER_MIWI)
        if (cbusNum == CBUS_OVER_MIWI)
        {

        }
    #endif

}


/*
 * Send a debug message with 5 data bytes
 */
void cbusSendDataEvent(BYTE cbusNum, WORD nodeID, BYTE *debug_data )
{
    BYTE msg[d7+1];

    msg[d0] = OPC_ACDAT;
    msg[d3] = debug_data[0];
    msg[d4] = debug_data[1];
    msg[d5] = debug_data[2];
    msg[d6] = debug_data[3];
    msg[d7] = debug_data[4];
    cbusSendMsgNN(cbusNum, nodeID, msg);

    #if defined(CBUS_OVER_CAN)
        if ((cbusNum == CBUS_OVER_CAN) || (cbusNum == 0xFF) )
            canQueueRx( msg );      // Queue event into receive buffer so module can be taugjt its own events

    #endif
}

		


