/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 can18.h - Definitions for CAN routines - part of CBUS libraries for PIC 18F

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


#ifndef __CANBUS_H
#define __CANBUS_H


#include "devincs.h"
#include "GenericTypeDefs.h"
#include "MoreTypes.h"
#include "hwsettings.h"
#include "TickTime.h"

#define DEFAULT_CANID   0x01                        // starting value, likely to be modified by auto conflict resolution
                                                    // Was originally 0x70 = 112 but supposed to be in range 1-99
                                                    // will cause a conflict but at least we can do auto conflict resolution
#define MAX_CANID       0x7F
#define ENUM_ARRAY_SIZE (MAX_CANID/8)+1              // Size of array for enumeration results
#define LARB_RETRIES    10                          // Number of retries for lost arbitration
#define CAN_TX_TIMEOUT  ONE_SECOND                  // Time for CAN transmit timeout (will resolve to one second intervals due to timer interrupt period)
#define ENUMERATION_TIMEOUT HUNDRED_MILI_SECOND     // Wait time for enumeration responses before setting canid
#define ENUMERATION_HOLDOFF 2 * HUNDRED_MILI_SECOND // Delay afer receiving conflict before initiating our own self enumeration

// Define sizes of additional software FIFOs
// Must be non-zero - a value of more than 16 will result in a buffer of more
// than 256 bytes, which will require a larger area definition in the link control 
// file and may  generate additional code from the compiler to manage the index values

#define CANTX_FIFO_LEN  16
#define CANRX_FIFO_LEN  16


// CANSTAT interrupt reason codes - not defined in processor header for some reason

#define IR_TXB0 0x08
#define IR_ERR  0x02


// CAN packet Buffer structure
// ??? Status (and pad) may no longer be required in this implementation, but leave them in place until we are sure after testing


enum CanBytes {
        con=0,
        sidh,
        sidl,
        eidh,
        eidl,
        dlc,
        d0,
        d1,
        d2,
        d3,
        d4,
        d5,
        d6,
        d7,
        pktsize
};

typedef struct {
  BYTE buffer[pktsize];
  BYTE status;
  BYTE pad;
} CanPacket;


#define ECAN_MSG_STD    0
#define ECAN_MSG_XTD    1

// Macros for chip family dependant register and bit locations

#ifdef CPUF18K
    #define TXBnIE      PIE5bits.TXBnIE
    #define TXBnIF      PIR5bits.TXBnIF
    #define ERRIE       PIE5bits.ERRIE
    #define ERRIF       PIR5bits.ERRIF
    #define FIFOWMIE    PIE5bits.FIFOWMIE
    #define FIFOWMIF    PIR5bits.FIFOWMIF
    #define RXBnIF      PIR5bits.RXBnIF
    #define IRXIF       PIR5bits.IRXIF
    #define RXBnOVFL    COMSTATbits.RXB1OVFL
#else
    #define TXBnIE      PIE3bits.TXBnIE
    #define TXBnIF      PIR3bits.TXBnIF
    #define ERRIE       PIE3bits.ERRIE
    #define ERRIF       PIR3bits.ERRIF
    #define FIFOWMIE    PIE3bits.FIFOWMIE
    #define FIFOWMIF    PIR3bits.FIFOWMIF
    #define RXBnIF      PIR3bits.RXBnIF
    #define IRXIF       PIR3bits.IRXIF
    #define RXBnOVFL    COMSTATbits.RXBnOVFL
#endif



extern BYTE clkMHz;

// Diagnostic variables for CAN performance

extern  BYTE  larbCount;
extern  BYTE  txErrCount;
extern  BYTE  txTimeoutCount;
extern  BYTE  maxCanTxFifo;
extern  BYTE  maxCanRxFifo;
extern  BYTE  txOflowCount;
extern  BYTE  rxOflowCount;
extern  BYTE  txFifoUsage;
extern  BYTE  rxFifoUsage;


void canInit(BYTE busNum, BYTE initCanID);
BOOL setNewCanId( BYTE newCanId );
BOOL canSend(BYTE *msg, BYTE msgLen);
BOOL canTX( CanPacket *msg );
BOOL canQueueRx( CanPacket *msg );
BOOL canbusRecv(CanPacket *msg);
void canFillRxFifo(void);
void checkTxFifo( void );
void checkCANTimeout( void );
void canTxError( void );
void canInterruptHandler( void );
void doEnum(BOOL sendResult);

#endif

