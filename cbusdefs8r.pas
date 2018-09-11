unit cbusdefs8n;

{ CBUS definitions for development in Delphi (Object Pascal)
  These definitions are taken from CBUS specifications document rev 8d embedded in developer guide 6b.


     (c) Pete Brownlow 2011-2015   software@upsys.co.uk
     Originally derived from opcodes.h (c) Andrew Crosland.

 This work is licensed under the:
       Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
       http://creativecommons.org/licenses/by-nc-sa/4.0/

 See the above link for full details

  For commercial use, please contact the original copyright holder(s) to agree licensing terms

 Pete Brownlow  6/4/11 - Original from spec version 7e
 Roger Healey   6/4/11 - Add OPC_CMDERR response codes
 Pete Brownlow  7/6/11 - Updated to spec ver 7f and add new module ids
 Pete Brownlow  4/7/11 - Updated to spec ver 7g
 Pete Brownlow  14/8/11 - Updated to spec ver 7h
 Pete Brownlow  18/2/12 - Updated to spec ver 8a, Rocrail and animated modeller module types added
 Pete Browlnow  17/4/12 - Updated parameter block definitions, added processor type definitions.
 Roger Healey   13/7/12 - Add CANTOTI
 Pete Brownlow  15/9/12 - Updated to spec ver 8c, added $5D ENUM and $75 CANID
 Pete Brownlow  4/1/13 -  Ver 8d - new module types, added $5E  NNRST
 Pete Brownlow  22/1/14 -  Ver 8g - new module types, OPC_NNRST & OPC_NNRSM, catch up with .inc file
 *                         Added parameter definitions for manufacturer's CPU id
 *                         Added extern C so can be included by c++ code
 Phil Wheeler       1/2/14     Added PRM_BETA
 Phil Wheeler     9/3/14 - Corrected CANSIG MTYP definitions
 Pete Brownlow   5/5/14 -
 Pete Brownlow 19/4/14 - Ver 8h - Added module type defs for CANACE8MIO, CANBIP, CANSOL
 Pete Brownlow 6/5/15 - Ver 8h - generate Delphi (object Pascal) version
 Pete Brownlow  06/7/15 - Ver 8j Add new module types as per modules definitions v17 : CANCDU, CANACC4CDU, CANWiBase, WiCAB, CANWiFi, CANFTT
                                 Add new opcode ALOC, added CPU manufacturer code CPUM_ATMEL
 Pete Brownlow  2/7/16 - Ver 8m   Add new module types CANHNDST, CANTCHNDST, CANRFID8, CANmchRFID and CANPiWi
                                  Add processor codes for ARM chips used in Raspberry Pi's
 Pete Brownlow	16/2/17- Ver 8n   Add module ids to bring it up to module ids document ver 25   
 Pete Brownlow	29/7/17 - Ver 8p   Add new module ids and new cab signalling opcode 
 Pete Brownlow	09/1/18 - Ver 8q   Add new parameter flag for module can consume its own events  
 Ian Hogg	11/09/18 - Ver 8r  Added CANACE16CMIO, CANPiNODE, CANDISP, CANCOMPUTE

}

interface

const

// Manufacturers

 MANU_MERG      =   165;    // http://www.merg.co.uk
 MANU_ROCRAIL   =   70;     // http://www.rocrail.net
 MANU_SPECTRUM  =   80;     // http://animatedmodeler.com  (Spectrum Engineering)

// MERG Module types

 MTYP_SLIM      =   0;      // default for SLiM nodes
 MTYP_CANACC4   =   1;      // Solenoid point driver
 MTYP_CANACC5   =   2;      // Motorised point driver
 MTYP_CANACC8   =   3;      // 8 digital outputs
 MTYP_CANACE3   =   4;      // Control panel switch/button encoder
 MTYP_CANACE8C  =   5;      // 8 digital inputs
 MTYP_CANLED    =   6;      // 64 led driver
 MTYP_CANLED64  =   7;      // 64 led driver (multi leds per event)
 MTYP_CANACC4_2 =   8;      // 12v version of CANACC4
 MTYP_CANCAB    =   9;      // CANCAB hand throttle
 MTYP_CANCMD    =   10;     // CANCMD command station
 MTYP_CANSERVO  =   11;     // 8 servo driver (on canacc8 or similar hardware)
 MTYP_CANBC     =   12;     // BC1a command station
 MTYP_CANRPI    =   13;     // RPI and RFID interface
 MTYP_CANTTCA   =   14;     // Turntable controller (turntable end)
 MTYP_CANTTCB   =   15;     // Turntable controller (control panel end)
 MTYP_CANHS     =   16;     // Handset controller for old BC1a type handsets
 MTYP_CANTOTI   =   17;     // Track occupancy detector
 MTYP_CAN8I8O   =   18;     // 8 inputs 8 outputs
 MTYP_CANSERVO8C=   19;     // Canservo with servo position feedback
 MTYP_CANRFID   =   20;     // RFID input
 MTYP_CANTC4    =   21;
 MTYP_CANACE16C =   22;     // 16 inputs
 MTYP_CANIO8    =   23;     // 8 way I/O
 MTYP_CANSNDX   =   24;     // ??
 MTYP_CANEther  =   25;     // Ethernet interface
 MTYP_CANSIG64  =   26;     // Multiple aspect signalling for CANLED module
 MTYP_CANSIG8   =   27;     // Multiple aspect signalling for CANACC8 module
 MTYP_CANCOND8C =   28;     // Conditional event generation
 MTYP_CANPAN    =   29;     // Control panel 32/32
 MTYP_CANACE3C  =   30;     // Newer version of CANACE3 firmware
 MTYP_CANPanel  =   31;     // Control panel 64/64
 MTYP_CANMIO    =   32;     // Multiple I/O
 TYP_CANACE8MIO =   33;     // Multiple IO module emulating ACE8C
 MTYP_CANSOL    =   34;     // Solenoid driver module
 MTYP_CANBIP    =   35;     // Bipolar IO module with additional 8 I/O pins
 MTYP_CANCDU    =   36;     // Solenoid driver module with additional 6 I/O pins
 MTYP_CANACC4CDU=   37;     // CANACC4 firmware ported to CANCDU
 MTYP_CANWiBase =   38;     // CAN to MiWi base station
 MTYP_WiCAB     =   39;     // Wireless cab using MiWi protocoal
 MTYP_CANWiFi   =   40;     // CAN to WiFi connection with Withrottle to CBUS protocol conversion
 MTYP_CANFTT    =   41;     // Turntable controller configured using FLiM
 MTYP_CANHNDST   =  42;
 MTYP_CANTCHNDST =  43;
 MTYP_CANRFID8   =  44;     // multi-channel RFID reader
 MTYP_CANmchRFID =  45;     // either a 2ch or 8ch RFID reader
 MTYP_CANPiWi    =  46;     //  a Raspberry Pi based module for WiFi
 MTYP_CAN4DC	 =  47;     //  DC train controller
 MTYP_CANELEV	 =  48;     //  Nelevator controller
 MTYP_CANSCAN	 =  49;     //  128 switch inputs
 MTYP_CANMIO_SVO =  50;     //  16MHz 25k80 version of CANSERVO8c
 MTYP_CANMIO_INP =  51;     //  16MHz 25k80 version of CANACE8MIO
 MTYP_CANMIO_OUT =  52;     //  16MHz 25k80 version of CANACC8
 MTYP_CANBIP_OUT =  53;     //  16MHz 25k80 version of CANACC5
 MTYP_CANASTOP	 =  54;     //  DCC stop generator 
 MTYP_CANCSB     =  55;     //  CANCMD with additional 3-4A booster on board
 MTYP_CANMAGOT	 =  56;     //  Magnet on Track detector 
 MTYP_CANCOMPUTE =  57;     //  Event processing engine
 MTYP_CANACE16CMIO =  57;     //  16 input equivaent to CANACE8C
 MTYP_CANPiNODE  =  58;     //  CBUS module based on Raspberry Pi
 MTYP_CANDISP    =  59;     //  25K80 version of CANLED64 (IHart and MB)
 MTYP_CANCOMPUTE =  60;     //  Event processing engine

  
 MTYP_CAN_SW    =   $FF;   // Software nodes
 MTYP_EMPTY     =   $FE;   // Empty module, bootloader only
 MTYP_CANUSB    =   $FD;   // USB interface

// Rocrail Module types

 MTYP_CANGC1    =   1;      // RS232 PC interface
 MTYP_CANGC2    =   2;      // 16 I/O
 MTYP_CANGC3    =   3;      // Command station (derived from cancmd)
 MTYP_CANGC4    =   4;      // 8 channel RFID reader
 MTYP_CANGC5    =   5;      // Cab for fixed panels (derived from cancab)
 MTYP_CANGC6    =   6;      // 4 channel servo controller
 MTYP_CANGC7    =   7;      // Fast clock module
 MTYP_CANGC1e   =   11;     // CAN<->Ethernet interface

// Animated Modeller module types

 MTYP_AMCTRLR   =   1;      // Animation controller (firmware derived from cancmd)
 MTYP_DUALCAB   =   2;      // Dual cab based on cancab


//  CBUS opcodes list

// Packets with no data bytes

 OPC_ACK        =   $00;    // General ack
 OPC_NAK        =   $01;    // General nak
 OPC_HLT        =   $02;    // Bus Halt
 OPC_BON        =   $03;    // Bus on
 OPC_TOF        =   $04;    // Track off
 OPC_TON        =   $05;    // Track on
 OPC_ESTOP      =   $06;    // Track stopped
 OPC_ARST       =   $07;    // System reset
 OPC_RTOF       =   $08;    // Request track off
 OPC_RTON       =   $09;    // Request track on
 OPC_RESTP      =   $0a;    // Request emergency stop all
 OPC_RSTAT      =   $0c;    // Request command station status
 OPC_QNN        =   $0d;    // Query nodes

 OPC_RQNP       =   $10;    // Read node parameters
 OPC_RQMN       =   $11;    // Request name of module type

// Packets with 1 data byte

 OPC_KLOC       =   $21;    // Release engine by handle
 OPC_QLOC       =   $22;    // Query engine by handle
 OPC_DKEEP      =   $23;    // Keep alive for cab

 OPC_DBG1       =   $30;    // Debug message with 1 status byte
 OPC_EXTC       =   $3F;    // Extended opcode

// Packets with 2 data bytes

 OPC_RLOC       =   $40;    // Request session for loco
 OPC_QCON       =   $41;    // Query consist
 OPC_SNN        =   $42;    // Set node number
 OPC_ALOC       =   $43;    // Allocate loco (used to allocate to a shuttle in cancmd)

 OPC_STMOD      =   $44;    // Set Throttle mode
 OPC_PCON       =   $45;    // Consist loco
 OPC_KCON       =   $46;    // De-consist loco
 OPC_DSPD       =   $47;    // Loco speed/dir
 OPC_DFLG       =   $48;    // Set engine flags
 OPC_DFNON      =   $49;    // Loco function on
 OPC_DFNOF      =   $4A;    // Loco function off
 OPC_SSTAT      =   $4C;    // Service mode status
 OPC_NNRSM      =   $4F;    // Reset to manufacturer defaults

 OPC_RQNN       =   $50;    // Request Node number in setup mode
 OPC_NNREL      =   $51;    // Node number release
 OPC_NNACK      =   $52;    // Node number acknowledge
 OPC_NNLRN      =   $53;    // Set learn mode
 OPC_NNULN      =   $54;    // Release learn mode
 OPC_NNCLR      =   $55;    // Clear all events
 OPC_NNEVN      =   $56;    // Read available event slots
 OPC_NERD       =   $57;    // Read all stored events
 OPC_RQEVN      =   $58;    // Read number of stored events
 OPC_WRACK      =   $59;    // Write acknowledge
 OPC_RQDAT      =   $5A;    // Request node data event
 OPC_RQDDS      =   $5B;    // Request short data frame
 OPC_BOOT       =   $5C;    // Put node into boot mode
 OPC_ENUM       =   $5D;    // Force can_id self enumeration
 OPC_NNRST      =   $5E;    // Reset node (as in restart)
 OPC_EXTC1      =   $5F;    // Extended opcode with 1 data byte

// Packets with 3 data bytes

 OPC_DFUN       =   $60;    // Set engine functions
 OPC_GLOC       =   $61;    // Get loco (with support for steal/share)
 OPC_ERR        =   $63;    // Command station error
 OPC_CMDERR     =   $6F;    // Errors from nodes during config

 OPC_EVNLF      =   $70;    // Event slots left response
 OPC_NVRD       =   $71;    // Request read of node variable
 OPC_NENRD      =   $72;    // Request read stored event by index
 OPC_RQNPN      =   $73;    // Request read module parameters
 OPC_NUMEV      =   $74;    // Number of events stored response
 OPC_CANID      =   $75;    // Set canid
 OPC_EXTC2      =   $7F;    // Extended opcode with 2 data bytes

// Packets with 4 data bytes

 OPC_RDCC3      =   $80;    // 3 byte DCC packet
 OPC_WCVO       =   $82;    // Write CV byte Ops mode by handle
 OPC_WCVB       =   $83;    // Write CV bit Ops mode by handle
 OPC_QCVS       =   $84;    // Read CV
 OPC_PCVS       =   $85;    // Report CV
 OPC_CABSIG     =   $86;    // Cab signalling    

 OPC_ACON       =   $90;    // on event
 OPC_ACOF       =   $91;    // off event
 OPC_AREQ       =   $92;    // Accessory Request event
 OPC_ARON       =   $93;    // Accessory response event on
 OPC_AROF       =   $94;    // Accessory response event off
 OPC_EVULN      =   $95;    // Unlearn event
 OPC_NVSET      =   $96;    // Set a node variable
 OPC_NVANS      =   $97;    // Node variable value response
 OPC_ASON       =   $98;    // Short event on
 OPC_ASOF       =   $99;    // Short event off
 OPC_ASRQ       =   $9A;    // Short Request event
 OPC_PARAN      =   $9B;    // Single node parameter response
 OPC_REVAL      =   $9C;    // Request read of event variable
 OPC_ARSON      =   $9D;    // Accessory short response on event
 OPC_ARSOF      =   $9E;    // Accessory short response off event
 OPC_EXTC3      =   $9F;    // Extended opcode with 3 data bytes

// Packets with 5 data bytes

 OPC_RDCC4      =   $A0;    // 4 byte DCC packet
 OPC_WCVS       =   $A2;    // Write CV service mode

 OPC_ACON1      =   $B0;    // On event with one data byte
 OPC_ACOF1      =   $B1;    // Off event with one data byte
 OPC_REQEV      =   $B2;    // Read event variable in learn mode
 OPC_ARON1      =   $B3;    // Accessory on response (1 data byte)
 OPC_AROF1      =   $B4;    // Accessory off response (1 data byte)
 OPC_NEVAL      =   $B5;    // Event variable by index read response
 OPC_PNN        =   $B6;    // Response to QNN
 OPC_ASON1      =   $B8;    // Accessory short on with 1 data byte
 OPC_ASOF1      =   $B9;    // Accessory short off with 1 data byte
 OPC_ARSON1     =   $BD;    // Short response event on with one data byte
 OPC_ARSOF1     =   $BE;    // Short response event off with one data byte
 OPC_EXTC4      =   $BF;    // Extended opcode with 4 data bytes

// Packets with 6 data bytes

 OPC_RDCC5      =   $C0;    // 5 byte DCC packet
 OPC_WCVOA      =   $C1;    // Write CV ops mode by address
 OPC_FCLK       =   $CF;    // Fast clock
        
 OPC_ACON2      =   $D0;    // On event with two data bytes
 OPC_ACOF2      =   $D1;    // Off event with two data bytes
 OPC_EVLRN      =   $d2;    // Teach event
 OPC_EVANS      =   $d3;    // Event variable read response in learn mode
 OPC_ARON2      =   $D4;    // Accessory on response
 OPC_AROF2      =   $D5;    // Accessory off response
 OPC_ASON2      =   $D8;    // Accessory short on with 2 data bytes
 OPC_ASOF2      =   $D9;    // Accessory short off with 2 data bytes
 OPC_ARSON2     =   $DD;    // Short response event on with two data bytes
 OPC_ARSOF2     =   $DE;    // Short response event off with two data bytes
 OPC_EXTC5      =   $DF;    // Extended opcode with 5 data bytes

// Packets with 7 data bytes

 OPC_RDCC6      =   $E0;    // 6 byte DCC packets
 OPC_PLOC       =   $E1;    // Loco session report
 OPC_NAME       =   $E2;    // Module name response
 OPC_STAT       =   $E3;    // Command station status report
 OPC_PARAMS     =   $EF;    // Node parameters response

 OPC_ACON3      =   $F0;    // On event with 3 data bytes
 OPC_ACOF3      =   $F1;    // Off event with 3 data bytes
 OPC_ENRSP      =   $F2;    // Read node events response
 OPC_ARON3      =   $F3;    // Accessory on response
 OPC_AROF3      =   $F4;    // Accessory off response
 OPC_EVLRNI     =   $F5;    // Teach event using event indexing
 OPC_ACDAT      =   $F6;    // Accessory data event: 5 bytes of node data (eg: RFID)
 OPC_ARDAT      =   $F7;    // Accessory data response
 OPC_ASON3      =   $F8;    // Accessory short on with 3 data bytes
 OPC_ASOF3      =   $F9;    // Accessory short off with 3 data bytes
 OPC_DDES       =   $FA;    // Short data frame aka device data event (device id plus 5 data bytes)
 OPC_DDRS       =   $FB;    // Short data frame response aka device data response
 OPC_ARSON3     =   $FD;    // Short response event on with 3 data bytes
 OPC_ARSOF3     =   $FE;    // Short response event off with 3 data bytes
 OPC_EXTC6      =   $FF;    // Extended opcode with 6 data byes


// Modes for STMOD

 TMOD_SPD_MASK  =   3;
 TMOD_SPD_128   =   0;
 TMOD_SPD_14    =   1;
 TMOD_SPD_28I   =   2;
 TMOD_SPD_28    =   3;

// Error codes for OPC_ERR

 ERR_LOCO_STACK_FULL    = 1;
 ERR_LOCO_ADDR_TAKEN    = 2;
 ERR_SESSION_NOT_PRESENT= 3;
 ERR_CONSIST_EMPTY      = 4;
 ERR_LOCO_NOT_FOUND     = 5;
 ERR_CMD_RX_BUF_OFLOW   = 6;
 ERR_INVALID_REQUEST    = 7;
 ERR_SESSION_CANCELLED  = 8;

// Status codes for OPC_SSTAT

 SSTAT_NO_ACK   =   1;
 SSTAT_OVLD     =   2;
 SSTAT_WR_ACK   =   3;
 SSTAT_BUSY     =   4;
 SSTAT_CV_ERROR =   5;

// Error codes for OPC_CMDERR

 CMDERR_INV_CMD         = 1;
 CMDERR_NOT_LRN         = 2;
 CMDERR_NOT_SETUP       = 3;
 CMDERR_TOO_MANY_EVENTS = 4;
 CMDERR_NO_EV           = 5;  
 CMDERR_INV_EV_IDX      = 6;
 CMDERR_INVALID_EVENT   = 7;
//CMDERR_INV_EN_IDX     = 8;    now reserved
 CMDERR_INV_PARAM_IDX   = 9;
 CMDERR_INV_NV_IDX      = 10;
 CMDERR_INV_EV_VALUE    = 11;
 CMDERR_INV_NV_VALUE    = 12;

// Aspect codes for OPC_CABSIG
	
; First aspect byte
	
SASP_DANGER         = 0;
SASP_CAUTION        = 1;
SASP_PRELIM_CAUTION = 2;
SASP_PROCEED        = 3;
SASP_CALLON         = 4;    // Set bit 2 for call-on - main aspect will usually be at danger
SASP_THEATRE        = 8;    // Set bit 3 to 0 for upper nibble is feather lcoation, set 1 for upper nibble is theatre code
	
; Second Aspect byte
	
SASP_LIT            = 0;    // Set bit 0 to indicate lit
SASP_LUNAR          = 1;    // Set bit 1 for lunar indication
	
; Remaining bits in second aspect byte yet to be defined - use for other signalling systems	


// Parameter index numbers (readable by OPC_RQNPN, returned in OPC_PARAN)
// Index numbers count from 1, subtract 1 for offset into parameter block
// Note that RQNPN with index 0 returns the parameter count
    
 PAR_MANU       = 1;    // Manufacturer id
 PAR_MINVER     = 2;    // Minor version letter
 PAR_MTYP       = 3;    // Module type code
 PAR_EVTNUM     = 4;    // Number of events supported
 PAR_EVNUM      = 5;    // Event variables per event
 PAR_NVNUM      = 6;    // Number of Node variables
 PAR_MAJVER     = 7;    // Major version number
 PAR_FLAGS      = 8;    // Node flags
 PAR_CPUID      = 9;    // Processor type
 PAR_BUSTYPE    = 10;   // Bus type
 PAR_LOAD       = 11;   // load address, 4 bytes
 PAR_CPUMID     = 15;   // CPU manufacturer's id as read from the chip config space, 4 bytes (note - read from cpu at runtime, so not included in checksum)
 PAR_CPUMAN     = 19;   // CPU manufacturer code
 PAR_BETA       = 20;   // Beta revision (numeric), or 0 if release

// Offsets to other values stored at the top of the parameter block.
// These are not returned by opcode PARAN, but are present in the hex
// file for FCU.

 PAR_COUNT      = $18;    // Number of parameters implemented
 PAR_NAME       = $1A;    // 4 byte Address of Module type name, up to 8 characters null terminated
 PAR_CKSUM      = $1E;    // Checksum word at end of parameters

// Flags in PAR_FLAGS

 PF_NOEVENTS    =   0;
 PF_CONSUMER    =   1;
 PF_PRODUCER    =   2;
 PF_COMBI       =   3;
 PF_FLiM        =   4;
 PF_BOOT        =   8;
 PF_COE			=   16;    // Module can consume its own events

// BUS type that module is connected to

 PB_CAN         = 1;
 PB_ETH         = 2;
 PB_MIWI        = 3;

// Processor manufacturer codes

 CPUM_MICROCHIP =         01;
 CPUM_ATMEL     =         02;
 CPU_ARM        =         03;

// Processor type codes (identifies to FCU for bootload compatiblity)

 P18F2480       = 1;
 P18F4480       = 2;
 P18F2580       = 3;
 P18F4580       = 4;
 P18F2585       = 5;
 P18F4585       = 6;
 P18F2680       = 7;
 P18F4680       = 8;
 P18F2682       = 9;
 P18F4682       = 10;
 P18F2685       = 11;
 P18F4685       = 12;

 P18F25K80      = 13;
 P18F45K80      = 14;
 P18F26K80      = 15;
 P18F46K80      = 16;
 P18F65K80      = 17;
 P18F66K80      = 18;

 P32MX534F064   =         30;
 P32MX564F064   =         31;
 P32MX564F128   =         32;
 P32MX575F256   =         33;
 P32MX575F512   =         34;
 P32MX764F128   =         35;
 P32MX775F256   =         36;
 P32MX775F512   =         37;
 P32MX795F512   =         38;


 // ARM Processor type codes (identifies to FCU for bootload compatiblity)

 ARM1176JZ_FS     = 1;   // As used in Raspberry Pi
 ARMCortex_A7   = 2;   // As Used in Raspberry Pi 2
 ARMCortex_A53  = 3;   // As used in Raspberry Pi 3

implementation

end.
