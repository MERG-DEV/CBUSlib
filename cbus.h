#ifndef __CBUS_H
#define __CBUS_H

/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 cbus.h - Definitions for CBUS routines - part of CBUS libraries for PIC 18F

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

#include "..\cbuslib\cbusdefs8n.h"
#include "hwsettings.h"
#include "cbusconfig.h"

#if defined(CBUS_OVER_CAN)
    #include "can18.h"
#endif

#if defined(CBUS_OVER_MIWI)
    #include "cbus2miwi.h"
#endif

#define ALL_CBUS    0xFF

extern WORD    nodeID;
extern BYTE    cbusMsg[pktsize];


void cbusInit( WORD initNodeID );
BOOL cbusMsgReceived( BYTE cbusNum, BYTE *msg );
void cbusSendSingleOpc(BYTE cbusNum, BYTE opc );
void cbusSendOpcMyNN(BYTE cbusNum, BYTE opc, BYTE *msg);
void cbusSendOpcNN(BYTE cbusNum, BYTE opc, WORD Node_id, BYTE *msg);
void cbusSendMsgMyNN(BYTE cbusNum, BYTE *msg);
void cbusSendMsgNN(BYTE cbusNum, WORD Node_id, BYTE *msg);
void cbusSendMsg(BYTE cbusNum, BYTE *msg);
void cbusSendMyEvent( BYTE cbusNum, WORD eventNum, BOOL onEvent );
void cbusSendEvent( BYTE cbusNum, WORD eventNode, WORD eventNum, BOOL onEvent );
void cbusSendEventWithData( BYTE cbusNum, WORD eventNode, WORD eventNum, BOOL onEvent, BYTE *msg, BYTE datalen );
void cbusSendDataEvent(BYTE cbusNum, WORD Node_id, BYTE *debug_data );


#endif
