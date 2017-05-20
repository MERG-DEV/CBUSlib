/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 CAN interface routines -  part of CBUS libraries for PIC 18F

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
#include "romops.h"
#include "EEPROM.h"


// The parameters passed include a bus number to support hardware with multiple interfaces,
// this version does not implemet multiple interfaces and ignores the busNum parameter


#include <can18.h>
#include <string.h>

#pragma udata CANTX_FIFO

far CanPacket canTxFifo[CANTX_FIFO_LEN];

#pragma udata CANRX_FIFO

far CanPacket canRxFifo[CANRX_FIFO_LEN];

#pragma udata

BYTE txIndexNextFree;
BYTE txIndexNextUsed;
BYTE rxIndexNextFree;
BYTE rxIndexNextUsed;

BYTE  larbRetryCount;
TickValue  canTransmitTimeout;
BOOL  canTransmitFailed;
BYTE  larbCount;
BYTE  txErrCount;
BYTE  txTimeoutCount;
BYTE  maxCanTxFifo;
BYTE  maxCanRxFifo;
BYTE  txOflowCount;
BYTE  rxOflowCount;
BYTE  txFifoUsage;
BYTE  rxFifoUsage;

TickValue   enumerationStartTime;
BOOL    enumerationRequired;
BOOL    enumerationInProgress;
BYTE    enumerationResults[ENUM_ARRAY_SIZE];

BYTE  canID;

//Internal routine definitions

static BYTE* _PointBuffer(BYTE b);
void processEnumeration(void);
BOOL checkIncomingPacket(CanPacket *ptr);
BOOL insertIntoRxFifo( CanPacket *ptr );


//*******************************************************************************
// Initialise CAN

void canInit(BYTE busNum, BYTE initCanID) {

  larbCount = 0;
  txErrCount = 0;
  txTimeoutCount = 0;
  canTransmitFailed = FALSE;
  canTransmitTimeout.Val = 0;
  maxCanTxFifo = 0;
  maxCanRxFifo = 0;
  rxOflowCount = 0;
  txOflowCount = 0;
  txIndexNextFree = 0;
  txIndexNextUsed = 0;
  rxIndexNextFree = 0;
  rxIndexNextUsed = 0;
  txFifoUsage = 0;
  rxFifoUsage = 0;


  // Put module into Configuration mode.
  CANCON = 0b10000000;
  
  // Wait for config mode
  while (CANSTATbits.OPMODE2 == 0);

  /*
   * The CAN buad rate pre-scaler is preset by the bootloader, so this code is written to be clock speed independant.
   * The global variable clkMHz is set by reading in CAN prescaler value from BRGCON or CiCFG for use by any other routines
   * that need to know the clock speed. Note that on the PIC32 the Perhiperal bus prescaler setting will also need to be taken
   * into accounte when configurig time related values for perhiperals that use pbclk.
   */

#ifdef __C32__

  clkMHz = (( CiCFG & 0x3F ) + 1 ) << 2;        // Convert CAN config register into clock MHz

#else

    ECANCON   = 0b10110000;   // ECAN mode 2 with FIFO, FIFOWM = 1 (int when four spaces left), init to first RX buffer
    BSEL0     = 0;            // Use all 8 buffers for receive FIFO, as they do not work as a FIFO for transmit

    #ifdef CPUF18F
      BRGCON1 = 0B00000011;                           // ??? Set for 16MHz clock for the moment, will be 64MHz when PLL used, then remove from here when set in bootloader
    #else
      BRGCON1 = 0b00001111;                         // 16MHz resonator + PLL = 64MHz clock
    #endif

  clkMHz = (( BRGCON1 & 0x3F ) + 1 ) << 2;      // Convert BRGCON1 value into clock MHz.

#endif  
  
  /*  The CAN bit rates used for CBUS are calculated as follows:
 
   * Sync segment is fixed at 1 Tq
   * We are using propogation time of 7tq, phase 1 of 4Tq and phase2 of 4Tq.
   * Total bit time is Sync + prop + phase 1 + phase 2
   * or 16 * Tq in our case
   * So, for 125kbits/s, bit time = 8us, we need Tq = 500ns
   * To get 500nS, we set the CAN bit rate prescaler, in BRGCON1, to half the FOsc clock rate.
   * For example, 16MHz oscillator using PLL, Fosc is 64MHz, Tosc is 15.625nS, so we use prescaler of 1:32 to give Tq of 500nS  (15.625 x 32)
   * Having set Tq to 500nS, all other CAN timings are relative to Tq, so do not need chainging with processor clock speed
   */

  // BRGCON values used are as follows, now preset in the bootloader to make this routine clock speed independant:
    
  //BRGCON1 = 0b00000011; // 4MHz resonator + PLL = 16MHz clock (or 16MHz resonator no PLL)
  //BRGCON1 = 0b00000111; // 8MHz resonator + PLL = 32MHz clock`
  //BRGCON1 = 0b00001111; // 16MHz resonator + PLL = 64MHz clock 
    
  BRGCON2 = 0b10011110; // freely programmable, sample once, phase 1 = 4xTq, prop time = 7xTq
  BRGCON3 = 0b00000011; // Wake-up enabled, wake-up filter not used, phase 2 = 4xTq
  
  // Continue CAN initialisation from where bootloader left off
  
  CIOCON    = 0b00100000;    // TX drives Vdd when recessive, CAN capture to CCP1 disabled



// Setup masks so all filter bits are ignored apart from EXIDEN
    RXM0SIDH = 0;
    RXM0SIDL = 0x08;
    RXM0EIDH = 0;
    RXM0EIDL = 0;
    RXM1SIDH = 0;
    RXM1SIDL = 0x08;
    RXM1EIDH = 0;
    RXM1EIDL = 0;

    // Set filter 0 for standard ID only to reject bootloader messages
    RXF0SIDL = 0x80;

    // Link all filters to RXB0 - maybe only neccessary to link 1
    RXFBCON0 = 0;
    RXFBCON1 = 0;
    RXFBCON2 = 0;
    RXFBCON3 = 0;
    RXFBCON4 = 0;
    RXFBCON5 = 0;
    RXFBCON6 = 0;
    RXFBCON7 = 0;

    // Link all filters to mask 0
    MSEL0 = 0;
    MSEL1 = 0;
    MSEL2 = 0;
    MSEL3 = 0;

//  RXM0SIDL = 0; // Clear all mask registers so we get all CAN messages
//  RXM0SIDH = 0; //
//  RXM1SIDL = 0;
//  RXM1SIDH = 0;
//
//  RXM0EIDL = 0;
//  RXM0EIDH = 0;
//  RXM1EIDL = 0;
//  RXM1EIDH = 0;
//
//  RXFCON0 = 0;  // Disable filters
//  RXFCON1 = 0;

  // Configure the buffers to receive all valid messages
  // RXB0CON = RXB1CON = B0CON = B1CON = B2CON = B3CON = B4CON = B5CON = 0;

   // Clear RXFUL bits
    RXB0CON = 0;
    RXB1CON = 0;
    B0CON = 0;
    B1CON = 0;
    B2CON = 0;
    B3CON = 0;
    B4CON = 0;
    B5CON = 0;

  BIE0 = 0;                 // No Rx buffer interrupts (but we do use the high water mark interrupt)
  TXBIEbits.TXB0IE = 1;     // Tx buffer interrupts from buffer 0 only
  TXBIEbits.TXB1IE = 0;
  TXBIEbits.TXB2IE = 0;
  CANCON = 0;               // Set normal operation mode

  if (initCanID == 0)
  {
      canID = ee_read( EE_CAN_ID );

      if (canID == 0xFFFF)
          canID = DEFAULT_CANID;
  }
  else // use value passed to this routine
  {
      canID = initCanID;
      ee_write(EE_CAN_ID, canID);
  }

  // Preload TXB0 with parameters ready for sending CBUS data packets

  TXB0CON = 0;
  TXB0CONbits.TXPRI0 = 0;                           // Set buffer priority, so will be sent after any self enumeration packets
  TXB0CONbits.TXPRI1 = 0;
  TXB0DLC = 0;                                      // Not RTR, payload length will be set by transmit routine
  TXB0SIDH = 0b10110000 | ((canID & 0x78) >>3);     // Set CAN priority and ms 4 bits of can id
  TXB0SIDL = (canID & 0x07) << 5;                   // LS 3 bits of can id and extended id to zero

  // Preload TXB1 with RTR frame to initiate self enumeration when required

  TXB1CON = 0;
  TXB1CONbits.TXPRI0 = 0;                           // Set buffer priority, so will be sent before any CBUS data packets but after any enumreration replies
  TXB1CONbits.TXPRI1 = 1;
  TXB1DLC = 0x40;                                   // RTR packet with zero payload
  TXB1SIDH = 0b10110000 | ((canID & 0x78) >>3);     // Set CAN priority and ms 4 bits of can id
  TXB1SIDL = TXB0SIDL;                              // LS 3 bits of can id and extended id to zero

  // Preload TXB2 with a zero length packet containing CANID for  use in self enumeration

  TXB2CON = 0;
  TXB2CONbits.TXPRI0 = 1;                           // Set high buffer priority, so will be sent before any CBUS packets
  TXB2CONbits.TXPRI1 = 1;
  TXB2DLC = 0;                                      // Not RTR, zero payload
  TXB2SIDH = 0b10110000 | ((canID & 0x78) >>3);     // Set CAN priority and ms 8 bits of can id
  TXB2SIDL = TXB0SIDL;                   // LS 3 bits of can id and extended id to zero

  // Initialise enumeration control variables

  enumerationRequired = enumerationInProgress = FALSE;
  enumerationStartTime.Val = tickGet();

  // Initialisation complete, enable CAN interrupts

  FIFOWMIE = 1;    // Enable Fifo 1 space left interrupt
  ERRIE = 1;       // Enable error interrupts

}

// Set a new can id

void setNewCanId( BYTE newCanId )

{
  TXB0SIDH &= 0b11110000;                // Clear canid bits
  TXB0SIDH |= ((newCanId & 0x78) >>3);  // Set new can id for CUBS packet transmissions
  TXB0SIDL = ( newCanId & 0x07) << 5;

  TXB1SIDH &= 0b11110000;                // Clear canid bits
  TXB1SIDH |= ((newCanId & 0x78) >>3);  // Set new can id for self enumeration frame transmission
  TXB1SIDL = TXB0SIDL;

  TXB2SIDH &= 0b11110000;               // Clear canid bits
  TXB2SIDH |= ((newCanId & 0x78) >>3);  // Set new can id for self enumeration frame transmission
  TXB2SIDL = TXB0SIDL;

  ee_write(EE_CAN_ID, newCanId );       // Update saved value
}


// Send a message from a buffer provided by the caller

BOOL canSend(BYTE *msg, BYTE msgLen)
{
 //   CanPacket TXPacket;
    BOOL success;

    // Copy bare message into CAN packet structure

//    memcpy(TXPacket.buffer+d0, msg+d0, msgLen);
    msg[dlc] = msgLen;
    success = canTX((CanPacket*)msg);
    return( success );   // Will need some sort of timeout or at least status update mechanism here - should be sorted by underlying transmit timeout.
    
}

// Transmit a packet - DLC must be set to packet length but other fields are set by this routine

BOOL canTX( CanPacket *msg )
{
  BYTE* ptr;
  BOOL  fullUp;
  BYTE hiIndex;

  msg->buffer[con] = 0;
  msg->buffer[dlc] &= 0x0F;  // Ensure not RTR
  msg->buffer[sidh] = 0b10110000 | ((canID & 0x78) >>3);
  msg->buffer[sidl] = (canID & 0x07) << 5;

  if (msg->buffer[dlc] > 8)
      msg->buffer[dlc] = 8;


  TXBnIE = 0;    // Disable transmit buffer interrupt whilst we fiddle with registers and fifo
 
  // On chip Transmit buffers do not work as a FIFO, so use just one buffer and implement a software fifo

  if (((txIndexNextUsed == txIndexNextFree) || canTransmitFailed) && (!TXB0CONbits.TXREQ))  // check if software fifo empty and transmit buffer ready
  {
     ptr = (BYTE*) & TXB0CON;
     memcpy(ptr, (void *) msg->buffer, msg->buffer[dlc] + 6);

     larbRetryCount = LARB_RETRIES;
     canTransmitTimeout.Val = tickGet();
     canTransmitFailed = FALSE;

     TXB0CONbits.TXREQ = 1;    // Initiate transmission
     fullUp = FALSE;
     TXBnIE = 1;
  }
  else  // load it into software fifo
  {
      if (!(fullUp = (txIndexNextFree == 0xFF)))
      {
        memcpy( canTxFifo[txIndexNextFree].buffer, msg->buffer, msg->buffer[dlc] + 6);
  
        if (++txIndexNextFree == CANTX_FIFO_LEN )
            txIndexNextFree = 0;

        if (txIndexNextUsed == txIndexNextFree) // check if fifo now full
            txIndexNextFree = 0xFF; // mark as full
      }
      else
        txOflowCount++;

      // Track buffer usage

      txFifoUsage++;
      hiIndex = ( txIndexNextFree < txIndexNextUsed ? txIndexNextFree + CANTX_FIFO_LEN : txIndexNextFree);
      if ((hiIndex - txIndexNextUsed) > maxCanTxFifo )
        maxCanTxFifo = hiIndex - txIndexNextUsed;
  }

  TXBnIE = 1;  // Enable transmit buffer interrupt
 
  return !fullUp;   // Return true for succesfully submitted for transmission
}


// Queue a packet into the receive buffer
// This is used to queue outgoing events back into the rx buffer so that the module
// can be taught its own events
// Note that message length must already be set in dlc byte - this is done my cansend which
// should be called to transmit the message before canQueueRx is called

BOOL canQueueRx( CanPacket *msg )

{
    FIFOWMIE = 0;   // Disable high water mark interrupt so nothing will fiddle with FIFO
    insertIntoRxFifo( msg );
    FIFOWMIE = 1;
}


// Called by ISR to handle tx buffer interrupt

void checkTxFifo( void )
{
    BYTE* ptr;

    canTransmitFailed = FALSE;
    TXBnIF = 0;                 // reset the interrupt flag
    
    if (!TXB0CONbits.TXREQ)
    {
        canTransmitTimeout.Val = 0;
        
        if (txIndexNextUsed != txIndexNextFree)   // If data waiting in software fifo, and buffer ready
        {
            ptr = (BYTE*) & TXB0CON;              // Dest is CAN transmit buffer
            memcpy(ptr, canTxFifo[txIndexNextUsed].buffer, canTxFifo[txIndexNextUsed].buffer[dlc] + 6);
            txFifoUsage--;

            larbRetryCount = LARB_RETRIES;
            canTransmitTimeout.Val = tickGet();
            canTransmitFailed = FALSE;

            TXB0CONbits.TXREQ = 1;    // Initiate transmission

            if (txIndexNextFree == 0xFF)
                txIndexNextFree = txIndexNextUsed; // clear full status

            if (++txIndexNextUsed == CANTX_FIFO_LEN )
                txIndexNextUsed = 0;

            TXBnIE = 1;  // enable transmit buffer interrupt
        }
        else
        {
            TXB0CON = 0;
            TXBnIF = 0;
            TXBnIE = 0;
        }
    }
    else
    {
        TXBnIF = 0;
        TXBnIE = 1;
    }

 

} // checkTxFifo

// Called by high priority ISR at 1 sec intervals to check for timeout

void checkCANTimeout( void )
{
    if (canTransmitTimeout.Val != 0)
        if (tickTimeSince(canTransmitTimeout) > CAN_TX_TIMEOUT)
        {    
            canTransmitFailed = TRUE;
            txTimeoutCount++;
            TXB0CONbits.TXREQ = 0;  // abort timed out packet
            checkTxFifo();          //  See if another packet is waiting to be sent
        }    
}


//*******************************************************************************
// Called by main loop to check for cbus messages received
// *msg points to a message buffer where the next message is placed
// Returns TRUE if a message was found

BOOL canbusRecv(CanPacket *msg)
{

    CanPacket   *ptr;
    BOOL        msgFound;
 
    FIFOWMIE = 0;  // Disable high watermark interrupt so ISR canot fiddle with FIFOs or enumeration map

    processEnumeration();  // Start or finish canid enumeration if required

 
    // Check for any messages in the software fifo, which the ISR will have filled if there has been a high watermark interrupt

    if (rxIndexNextUsed != rxIndexNextFree)
    {
      memcpy(msg->buffer, canRxFifo[rxIndexNextUsed].buffer, canRxFifo[rxIndexNextUsed].buffer[dlc] + 6);
      rxFifoUsage--;
      
      if (++rxIndexNextUsed >= CANRX_FIFO_LEN)
      {
            rxIndexNextUsed = 0;
      }
      msgFound = TRUE;
    }
    else  // Nothing in software FIFO, so now check for message in hardware FIFO
    {
        msgFound = FALSE;

//        while (!msgFound && COMSTATbits.NOT_FIFOEMPTY)
        if (COMSTATbits.NOT_FIFOEMPTY)
        {
            ptr = (CanPacket*) _PointBuffer(CANCON & 0x07);
            RXBnIF = 0;
//            if (COMSTATbits.RXBnOVFL) {
//              maxcanq++; // Buffer Overflow
//              led3timer = 5;
//              LED3 = LED_OFF;
//              COMSTATbits.RXBnOVFL = 0;
//            }

            // Check incoming Canid and initiate self enumeration if it is the same as our own

            if (msgFound = checkIncomingPacket(ptr))
              memcpy(msg->buffer, (void*) ptr, ptr->buffer[dlc] + 6);  // Get message for processing

            // Record and Clear any previous invalid message bit flag.
            if (IRXIF) {
              IRXIF = 0;
            }
            // Mark that this buffer is read and empty.
            ptr->buffer[con] &= 0x7f;    // clear RXFUL bit

                //TODO - LED flickering to show CBUS activity level
                // led1timer = 2;
                // LED1 = LED_ON;
        } // while !msgFound
    }

    FIFOWMIE = 1; // Re-enable FIFO interrupts now out of critical section
    return msgFound;
}

// **************************************************************************
// Insert a CAN packet into the next free location of the receive FIFO

BOOL insertIntoRxFifo( CanPacket *ptr )

{
    memcpy(canRxFifo[rxIndexNextFree].buffer, ptr, ptr->buffer[dlc] + 6);
    rxFifoUsage++;

    if (++rxIndexNextFree >= CANRX_FIFO_LEN)
    {
      rxIndexNextFree = 0;
    }

    if (rxIndexNextFree == rxIndexNextUsed)
    {
        rxOflowCount++; // Buffer Overflow
        if (rxIndexNextFree == 0)
            rxIndexNextFree = CANRX_FIFO_LEN - 1;     // On overflow, received packets overwrite last received packet
        else
            rxIndexNextFree--;

    }
} // Insert into RX FIFO


// **********************************************************************************
// Called from isr when high water mark interrupt received
// Clears ECAN fifo into software FIFO

void canFillRxFifo(void)
{
  CanPacket *ptr;
  BYTE  hiIndex;


  while (COMSTATbits.NOT_FIFOEMPTY)
  {

    ptr = (CanPacket*) _PointBuffer(CANCON & 0x07);
    RXBnIF = 0;
    if (RXBnOVFL) {
   //   maxcan++; // Buffer Overflow
   //   led3timer = 5;
   //   LED3 = LED_OFF;
      RXBnOVFL = 0;
    }

    if (checkIncomingPacket(ptr))
    {
        insertIntoRxFifo( ptr );

        //   led3timer = 5;
        //   LED3 = LED_OFF;
    }

    // Record and Clear any previous invalid message bit flag.
    if (IRXIF) 
      IRXIF = 0;
    
    // Mark that this buffer is read and empty.
    ptr->buffer[con] &= 0x7f;
 
  //  led1timer = 2;
  //  LED1 = LED_ON;

    hiIndex = ( rxIndexNextFree < rxIndexNextUsed ? rxIndexNextFree + CANRX_FIFO_LEN : rxIndexNextFree);
    if ((hiIndex - rxIndexNextUsed) > maxCanRxFifo )
        maxCanRxFifo = hiIndex - rxIndexNextUsed;

  }  // While hardware FIFO not empty
  FIFOWMIF = 0;
} // canFillRxFifo


//***********************************************************************************************
// Check if enumeration pending, if so kick it off providing hold off time has expired
// If enumeration complete, find and set new can id

void processEnumeration(void)

{
    BYTE i, newCanId, enumResult;

    if (enumerationRequired && (tickTimeSince(enumerationStartTime) > ENUMERATION_HOLDOFF ))
    {
        for (i=0; i< ENUM_ARRAY_SIZE; i++)
            enumerationResults[i] = 0;
        enumerationResults[0] = 1;  // Don't allocate canid 0

        enumerationInProgress = TRUE;
        enumerationRequired = FALSE;
        enumerationStartTime.Val = tickGet();
        TXB1CONbits.TXREQ = 1;              // Send RTR frame to initiate self enumeration
    }
    else if (enumerationInProgress && (tickTimeSince(enumerationStartTime) > ENUMERATION_TIMEOUT ))
    {
        // Enumeration complete, find first free canid

        for (i=0; (enumerationResults[i] == 0xFF) && (i < ENUM_ARRAY_SIZE); i++);   // Find byte in array with first free flag

        if ((enumResult = enumerationResults[i]) != 0xFF)
        {
            for (newCanId = i*8; (enumResult & 0x01); newCanId++)
                enumResult >>= 1;
            canID = newCanId;
            setNewCanId(canID);
        }
        else //TODO  send error segment full packet  - and presumably just keep current CANID?
        {
        }
        enumerationInProgress = FALSE;

    }
}  // Process enumeration


//*******************************************************************************
// Check incoming packet for RTR enumeration request, any canid clash and zero payload
// Fills in bitmap with canid if self enumeration in progress
// Returns TRUE if packet is to be processed as a CBUS message

BOOL checkIncomingPacket(CanPacket *ptr)

{
    BYTE        incomingCanId;
    BOOL        msgFound;

    msgFound = FALSE;
    incomingCanId = (ptr->buffer[sidh] << 3) + (ptr->buffer[sidl] >> 5);

    if (enumerationInProgress)
        arraySetBit( enumerationResults, incomingCanId);
    else if (!enumerationRequired && (incomingCanId == canID))    
    {
        // If we receive a packet with our own canid, initiate enumeration as automatic conflict resolution (Thanks to Bob V for this idea)
        enumerationRequired = TRUE;
        enumerationStartTime.Val = tickGet();  // Start hold off time for self enumeration
    }

    // Check for RTR - self enumeration request from another module

    if (ptr->buffer[dlc] & 0x40 ) // RTR bit set?
    {
        TXB2CONbits.TXREQ = 1;                  // Send enumeration response (zero payload frame preloaded in TXB2)
        enumerationStartTime.Val = tickGet();   // re-Start hold off time for self enumeration
    }
    else
    {
        msgFound = ptr->buffer[dlc] & 0x0F;     // Check not zero payload
        if  (ptr->buffer[dlc] > 8)
            ptr->buffer[dlc] = 8; // Limit buffer size to 8 bytes (defensive coding - it should not be possible for it to ever be more than 8, but just in case
    }

    return( msgFound );
}


//****************************************************************************
// Process transmit error interrupt

void canTxError( void )
{
    if (TXB0CONbits.TXLARB) {  // lost arbitration
        if (larbRetryCount == 0) {	// already tried higher priority
            canTransmitFailed = TRUE;
            canTransmitTimeout.Val = 0;

            TXB0CONbits.TXREQ = 0;
            larbCount++;
        }
        else if ( --larbRetryCount == 0) {	// Allow tries at lower level priority first
            TXB0CONbits.TXREQ = 0;
            TXB0SIDH &= 0b00111111; 		// change to high priority  ?? check priority bits usage
            TXB0CONbits.TXREQ = 1;			// try again
        }
    }
    if (TXB0CONbits.TXERR) {	// bus error
      canTransmitFailed = TRUE;
      canTransmitTimeout.Val = 0;
      TXB0CONbits.TXREQ = 0;
      txErrCount++;
    }
    
    if (canTransmitFailed)
        checkTxFifo();  // Check to see if more to try and send

    ERRIF = 0;
}


// This routine is called to manage the CAN interrupts.
// It may be called directly from the ISR definition in the application, or it may be called from the MLA interrupt handler
// via the applicationInterruptHandler routine


void canInterruptHandler( void )
{
    if (FIFOWMIF)       // Receive buffer high water mark, so move data into software fifo
        canFillRxFifo();

    if (ERRIF)
        canTxError();

    if (TXBnIF)
        checkTxFifo();

    checkCANTimeout();
}


// Set pointer to correct receive register set for incoming packet

static BYTE* _PointBuffer(BYTE b) {
  BYTE* pt;

  switch (b) {
    case 0:
      pt = (BYTE*) & RXB0CON;
      break;
    case 1:
      pt = (BYTE*) & RXB1CON;
      break;
    case 2:
      pt = (BYTE*) & B0CON;
      break;
    case 3:
      pt = (BYTE*) & B1CON;
      break;
    case 4:
      pt = (BYTE*) & B2CON;
      break;
    case 5:
      pt = (BYTE*) & B3CON;
      break;
    case 6:
      pt = (BYTE*) & B4CON;
      break;
    default:
      pt = (BYTE*) & B5CON;
      break;
  }
  return (pt);
}
