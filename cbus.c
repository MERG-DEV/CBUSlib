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

     Ported to XC8 compiler by Ian Hogg

*/
/**
 * Routines to support CBUS over CAN or MIWI.
 */

#include "can18.h"


#include "GenericTypeDefs.h"
#include "cbus.h"
#include "romops.h"
#include "EEPROM.h"

WORD    nodeID;
BYTE    cbusMsg[sizeof(CanPacket)]; // Global buffer for fast access to CBUS packets - do NOT use in ISRs as would not be re-entrant
#ifndef __XC8__
//#pragma code APP
#endif


/**
 *  Initialise CBUS  - Note: CANID only used when bus type is CAN.
 * 
 * @param initNodeID the CANID
 */
void cbusInit( WORD initNodeID  )
{
    
    nodeID = ee_read_short( (WORD)EE_NODE_ID );

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


/**
 * Check for CBUS message received.
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param msg location of received message
 * @return TRUE with message in msg buffer if a message has been received
 */
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
    return FALSE;
}

/**
 * Send single byte frame - opcode only
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param opc the CBUS opcode to be sent
 * @return true if sent ok
 */
BOOL cbusSendSingleOpc(BYTE cbusNum, BYTE opc )
{
    cbusMsg[d0] = opc;
    return cbusSendMsg(cbusNum, cbusMsg);
}



/**
 * Send opcode plus our node number, can be used to send simple 3-byte frame, or may have further 
 * bytes prepared in the buffer by the caller.
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param opc the CBUS opcode to be sent
 * @param msg the CBUS message to be sent
 * @return true if sent ok
 */
BOOL cbusSendOpcMyNN(BYTE cbusNum, BYTE opc, BYTE *msg)
{
    msg[d0] = opc;
    return cbusSendMsgMyNN(cbusNum, msg);
}


/**
 * Send opcode plus specified node number, can be used to send simple 3-byte frame, or may have further 
 * bytes prepared in the buffer by the caller.
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param opc the CBUS opcode to be sent
 * @param nodeID the CBUS node number to be sent
 * @param msg the CBUS message to be sent
 * @return true if sent ok
 */
BOOL cbusSendOpcNN(BYTE cbusNum, BYTE opc, WORD nodeID, BYTE *msg)
{
	msg[d0] = opc;
	return cbusSendMsgNN(cbusNum, nodeID, msg);
}


/**
 *  Send long CBUS event, no data, using our node number.
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param eventNum the CBUS event EN
 * @param onEvent if true ON event otherwise OFF event is sent 
 * @return true if sent ok
 */
BOOL cbusSendMyEvent( BYTE cbusNum, WORD eventNum, BOOL onEvent )
{
    BYTE    msg[d0+4];

    return cbusSendEventWithData( cbusNum, -1, eventNum, onEvent, msg, 0);
}

/**
 * Send CBUS event, no data, using specified node number.
 * Send short event if eventNode is 0
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param eventNode the event node number. 0 for a short event
 * @param eventNum the CBUS event EN
 * @param onEvent if true ON event otherwise OFF event is sent 
 * @return true if sent ok
 */
BOOL cbusSendEvent( BYTE cbusNum, WORD eventNode, WORD eventNum, BOOL onEvent )
{
    BYTE    msg[d0+4];

    return cbusSendEventWithData( cbusNum, eventNode, eventNum, onEvent, msg, 0);
}



/**
 * Send CBUS event with optional data bytes - works out appropriate opcode.
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param eventNode the event node number. 0 for a short event
 * @param eventNum the CBUS event EN
 * @param onEvent if true ON event otherwise OFF event is sent 
 * @param msg the CBUS message data byes to be sent
 * @param the number of data bytes to be sent
 * @return true if sent ok
 */
BOOL cbusSendEventWithData( BYTE cbusNum, WORD eventNode, WORD eventNum, BOOL onEvent, BYTE *msg, BYTE datalen )
{
    BOOL ret;
    msg[d0] = OPC_ACON;         // Start with long event opcode
    
    if (eventNode == 0)
    {
        msg[d0] |= 0x08;        // Short event opcode
        eventNode = nodeID;     // Add module node id for diagnostics
    }

    if (!onEvent)
        msg[d0] |= 0x01;        // Off event

    if (datalen > 0)
        msg[d0] |= (unsigned char)(datalen << 5U);   // Opcode for event + data

    msg[d3] = eventNum>>8;
    msg[d4] = eventNum & 0xFF;

    ret = cbusSendMsgNN( cbusNum, eventNode, msg );

    #if defined(CBUS_OVER_CAN)
        if ((cbusNum == CBUS_OVER_CAN) || (cbusNum == 0xFF) )
            canQueueRx( (CanPacket*)msg );      // Queue event into receive buffer so module can be taught its own events

    #endif
    return ret;
} // cbusSendEventWithData

/**
 * Send a CBUS message, putting our Node Number in first two message bytes
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param msg the CBUS message to be sent
 * @return true if sent ok
 */
BOOL cbusSendMsgMyNN(BYTE cbusNum, BYTE *msg)
{
    msg[d1] = nodeID>>8;
    msg[d2] = nodeID & 0xFF;
    return cbusSendMsg(cbusNum, msg);
}

/**
 * Send a CBUS message, work out node Number and put it in first two message bytes.
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param eventNode Bytes added to message after our node Number
 * @param msg the CBUS message to be sent
 * @return true if sent ok
 */
BOOL cbusSendMsgNN(BYTE cbusNum, WORD eventNode, BYTE *msg)
{
    if (eventNode == -1)
        eventNode = nodeID; // Use node id for this module

    msg[d1] = eventNode>>8;
    msg[d2] = eventNode & 0xFF;
    return cbusSendMsg(cbusNum, msg);
}


/**
 * Send a CBUS message where data bytes have already been loaded
 * Works out data length from opcode
 * If cbusNum is set to 0xFF, transmits on all defined CBUS connections
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param msg the CBUS message to be sent
 * @return true if sent ok
 */
BOOL cbusSendMsg(BYTE cbusNum, BYTE *msg)
{
    #if defined(CBUS_OVER_CAN)
        if ((cbusNum == CBUS_OVER_CAN) || (cbusNum == 0xFF) )
            return canSend( msg, (unsigned char)((unsigned char)(msg[d0] >> 5U)+1U));	// data length from opcode

    #endif


    #if defined(CBUS_OVER_MIWI)
        if (cbusNum == CBUS_OVER_MIWI)
        {

        }
    #endif
        return TRUE;
}


/**
 * Send a debug message with 5 data bytes.
 * 
 * @param cbusNum whether CAN or MIWI bus is to be checked.
 * @param nodeID The bytes to be be sent in the NN position
 * @param debug_data the CBUS message to be sent
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
            canQueueRx((CanPacket*) msg );      // Queue event into receive buffer so module can be taugjt its own events

    #endif
}

		


