package jmri.jmrix.can.cbus;

/**
 * CbusDefs.java rev 8q
 *
 * Description: Constants to represent CBUS protocol
 *
 * @author Andrew Crosland Copyright (C) 2008
 * Updated to Ver 8q Pete Brownlow 27-Jan-18
 * Updated to Ver 8r Ian Hogg 11-Sep-18. Added CANACE16CMIO, CANPiNODE, CANDISP, CANCOMPUTE
 * This class is the software and platform independent CBUS definitions
 * JMRI specific CBUS definitions are still in CbusConstants
 */
public final class CbusDefs {

    /**
     * CBUS Manufacturer definitions
     * Where the manufacturer already has an NMRA code, this is used
     */
    
    public static final int CBUS_MANU_MERG      =   165;    // http://www.merg.co.uk
    public static final int CBUS_MANU_ROCRAIL   =   70;     // http://www.rocrail.net
    public static final int CBUS_MANU_SPECTRUM  =   80;     // http://animatedmodeler.com  (Spectrum Engineering)

// MERG Module types

    public static final int CBUS_MTYP_SLIM      =   0;      // default for SLiM nodes
    public static final int CBUS_MTYP_CANACC4   =   1;      // Solenoid point driver
    public static final int CBUS_MTYP_CANACC5   =   2;      // Motorised point driver
    public static final int CBUS_MTYP_CANACC8   =   3;      // 8 digital outputs
    public static final int CBUS_MTYP_CANACE3   =   4;      // Control panel switch/button encoder
    public static final int CBUS_MTYP_CANACE8C  =   5;      // 8 digital inputs
    public static final int CBUS_MTYP_CANLED    =   6;      // 64 led driver
    public static final int CBUS_MTYP_CANLED64  =   7;      // 64 led driver (multi leds per event)
    public static final int CBUS_MTYP_CANACC4_2 =   8;      // 12v version of CANACC4
    public static final int CBUS_MTYP_CANCAB    =   9;      // CANCAB hand throttle
    public static final int CBUS_MTYP_CANCMD    =   10;     // CANCMD command station
    public static final int CBUS_MTYP_CANSERVO  =   11;     // 8 servo driver (on canacc8 or similar hardware)
    public static final int CBUS_MTYP_CANBC     =   12;     // BC1a command station
    public static final int CBUS_MTYP_CANRPI    =   13;     // RPI and RFID interface
    public static final int CBUS_MTYP_CANTTCA   =   14;     // Turntable controller (turntable end)
    public static final int CBUS_MTYP_CANTTCB   =   15;     // Turntable controller (control panel end)
    public static final int CBUS_MTYP_CANHS     =   16;     // Handset controller for old BC1a type handsets
    public static final int CBUS_MTYP_CANTOTI   =   17;     // Track occupancy detector
    public static final int CBUS_MTYP_CAN8I8O   =   18;     // 8 inputs 8 outputs
    public static final int CBUS_MTYP_CANSERVO8C=   19;     // Canservo with servo position feedback
    public static final int CBUS_MTYP_CANRFID   =   20;     // RFID input
    public static final int CBUS_MTYP_CANTC4    =   21;
    public static final int CBUS_MTYP_CANACE16C =   22;     // 16 inputs
    public static final int CBUS_MTYP_CANIO8    =   23;     // 8 way I/O
    public static final int CBUS_MTYP_CANSNDX   =   24;     // ??
    public static final int CBUS_MTYP_CANEther  =   25;     // Ethernet interface
    public static final int CBUS_MTYP_CANSIG64  =   26;     // Multiple aspect signalling for CANLED module
    public static final int CBUS_MTYP_CANSIG8   =   27;     // Multiple aspect signalling for CANACC8 module
    public static final int CBUS_MTYP_CANCOND8C =   28;     // Conditional event generation
    public static final int CBUS_MTYP_CANPAN    =   29;     // Control panel 32/32
    public static final int CBUS_MTYP_CANACE3C  =   30;     // Newer version of CANACE3 firmware
    public static final int CBUS_MTYP_CANPanel  =   31;     // Control panel 64/64
    public static final int CBUS_MTYP_CANMIO    =   32;     // Multiple I/O
    public static final int CBUS_MTYP_CANACE8MIO =  33;     // Multiple IO module emulating ACE8C
    public static final int CBUS_MTYP_CANSOL    =   34;     // Solenoid driver module
    public static final int CBUS_MTYP_CANBIP    =   35;     // Bipolar IO module with additional 8 I/O pins
    public static final int CBUS_MTYP_CANCDU    =   36;     // Solenoid driver module with additional 6 I/O pins
    public static final int CBUS_MTYP_CANACC4CDU=   37;     // CANACC4 firmware ported to CANCDU
    public static final int CBUS_MTYP_CANWiBase =   38;     // CAN to MiWi base station
    public static final int CBUS_MTYP_WiCAB     =   39;     // Wireless cab using MiWi protocoal
    public static final int CBUS_MTYP_CANWiFi   =   40;     // CAN to WiFi connection with Withrottle to CBUS protocol conversion
    public static final int CBUS_MTYP_CANFTT    =   41;     // Turntable controller configured using FLiM
    public static final int CBUS_MTYP_CANHNDST   =  42;
    public static final int CBUS_MTYP_CANTCHNDST =  43;
    public static final int CBUS_MTYP_CANRFID8   =  44;     // multi-channel RFID reader
    public static final int CBUS_MTYP_CANmchRFID =  45;     // either a 2ch or 8ch RFID reader
    public static final int CBUS_MTYP_CANPiWi    =  46;     //  a Raspberry Pi based module for WiFi
    public static final int CBUS_MTYP_CAN4DC	 =  47;     //  DC train controller
    public static final int CBUS_MTYP_CANELEV	 =  48;     //  Nelevator controller
    public static final int CBUS_MTYP_CANSCAN	 =  49;     //  128 switch inputs
    public static final int CBUS_MTYP_CANMIO_SVO =  50;     //  16MHz 25k80 version of CANSERVO8c
    public static final int CBUS_MTYP_CANMIO_INP =  51;     //  16MHz 25k80 version of CANACE8MIO
    public static final int CBUS_MTYP_CANMIO_OUT =  52;     //  16MHz 25k80 version of CANACC8
    public static final int CBUS_MTYP_CANBIP_OUT =  53;     //  16MHz 25k80 version of CANACC5
    public static final int CBUS_MTYP_CANASTOP	 =  54;     //  DCC stop generator 
    public static final int CBUS_MTYP_CANCSB     =  55;     //  CANCMD with additional 3-4A booster on board
    public static final int CBUS_MTYP_CANMAGOT	 =  56;     //  Magnet on Track detector 
    public static final int CBUS_MTYP_CANACE16CMIO =  57;     //  16 input equivaent to CANACE8C
    public static final int CBUS_MTYP_CANPiNODE  =  58;     //  CBUS module based on Raspberry Pi
    public static final int CBUS_MTYP_CANDISP    =  59;     //  25K80 version of CANLED64 (IHart and MB)
    public static final int CBUS_MTYP_CANCOMPUTE =  60;     //  Event processing engine
    public static final int CBUS_MTYP_CAN_SW    =   0xFF;   // Software nodes
    public static final int CBUS_MTYP_EMPTY     =   0xFE;   // Empty module, bootloader only
    public static final int CBUS_MTYP_CANUSB    =   0xFD;   // USB interface
    
    // Rocrail Module types
    public static final int CBUS_MTYP_CANGC1    =   1;      // RS232 PC interface
    public static final int CBUS_MTYP_CANGC2    =   2;      // 16 I/O
    public static final int CBUS_MTYP_CANGC3    =   3;      // Command station (derived from cancmd)
    public static final int CBUS_MTYP_CANGC4    =   4;      // 8 channel RFID reader
    public static final int CBUS_MTYP_CANGC5    =   5;      // Cab for fixed panels (derived from cancab)
    public static final int CBUS_MTYP_CANGC6    =   6;      // 4 channel servo controller
    public static final int CBUS_MTYP_CANGC7    =   7;      // Fast clock module
    public static final int CBUS_MTYP_CANGC1e   =   11;     // CAN<->Ethernet interface
    
    // Animated Modeller module types
    
    public static final int CBUS_MTYP_AMCTRLR   =   1;      // Animation controller (firmware derived from cancmd)
    public static final int CBUS_MTYP_DUALCAB   =   2;      // Dual cab based on cancab    
    
    /**
     * CBUS Opcodes
     */
    // Opcodes with no data bytes
    
    public static final int CBUS_ACK = 0x00;		// General ack
    public static final int CBUS_NAK = 0x01;        // General nak
    public static final int CBUS_HLT = 0x02;        // Bus Halt
    public static final int CBUS_BON = 0x03;        // Bus on
    public static final int CBUS_TOF = 0x04;        // Track off
    public static final int CBUS_TON = 0x05;        // Track on
    public static final int CBUS_ESTOP = 0x06;      // Track stopped
    public static final int CBUS_ARST = 0x07;       // System reset
    public static final int CBUS_RTOF = 0x08;       // Request track off
    public static final int CBUS_RTON = 0x09;       // Request track on
    public static final int CBUS_RESTP = 0x0A;      // Request emergency stop all
                                                    
    public static final int CBUS_RSTAT = 0x0C;      // Request command station status 
    public static final int CBUS_QNN = 0x0D;        // Query nodes

    public static final int CBUS_RQNP = 0x10;		// Read node parameters
    public static final int CBUS_RQMN = 0x11;       // Request name of module type

    // Opcodes with 1 data byte
    
    public static final int CBUS_KLOC = 0x21;		// Release engine by handle
    public static final int CBUS_QLOC = 0x22;       // Query engine by handle
    public static final int CBUS_DKEEP = 0x23;      // Keep alive for cab
                                                    
    public static final int CBUS_DBG1 = 0x30;       // Debug message with 1 status byte

    public static final int CBUS_EXTC = 0x3F;       // Extended opcode

    // Opcodes with 2 data
    
    public static final int CBUS_RLOC = 0x40;       // Request session for loco
    public static final int CBUS_QCON = 0x41;       // Query consist
    public static final int CBUS_SNN = 0x42;        // Set node number
    public static final int CBUS_ALOC = 0x43;       // Allocate loco (used to allocate to a shuttle in cancmd)
    public static final int CBUS_STMOD = 0x44;      // Set Throttle mode
    public static final int CBUS_PCON = 0x45;       // Consist loco
    public static final int CBUS_KCON = 0x46;       // De-consist loco
    public static final int CBUS_DSPD = 0x47;       // Loco speed/dir
    public static final int CBUS_DFLG = 0x48;       // Set engine flags
    public static final int CBUS_DFNON = 0x49;      // Loco function on
    public static final int CBUS_DFNOF = 0x4A;      // Loco function off
    public static final int CBUS_SSTAT = 0x4C;      // Service mode status
    public static final int CBUS_NNRSM = 0x4F;      // Reset to manufacturer defaults

    public static final int CBUS_RQNN = 0x50;       // Request Node number in setup mode
    public static final int CBUS_NNREL = 0x51;      // Node number release
    public static final int CBUS_NNACK = 0x52;      // Node number acknowledge
    public static final int CBUS_NNLRN = 0x53;      // Set learn mode
    public static final int CBUS_NNULN = 0x54;      // Release learn mode
    public static final int CBUS_NNCLR = 0x55;      // Clear all events
    public static final int CBUS_NNEVN = 0x56;      // Read available event slots
    public static final int CBUS_NERD = 0x57;       // Read all stored events
    public static final int CBUS_RQEVN = 0x58;      // Read number of stored events
    public static final int CBUS_WRACK = 0x59;      // Write acknowledge
    public static final int CBUS_RQDAT = 0x5A;      // Request node data event
    public static final int CBUS_RQDDS = 0x5B;      // Request short data frame
    public static final int CBUS_BOOTM = 0x5C;      // Put node into boot mode
    public static final int CBUS_ENUM = 0x5D;       // Force can_id self enumeration
    public static final int CBUS_NNRST = 0x5E;      // Reset node (as in restart)
    public static final int CBUS_EXTC1 = 0x5F;      // Extended opcode with 1 data byte

    // Opcodes with 3 data bytes
    
    public static final int CBUS_DFUN = 0x60;       // Set engine functions
    public static final int CBUS_GLOC = 0x61;       // Get loco (with support for steal/share)
    public static final int CBUS_ERR = 0x63;        // Command station error

    public static final int CBUS_CMDERR = 0x6F;     // Errors from nodes during config

    public static final int CBUS_EVNLF = 0x70;      // Event slots left response
    public static final int CBUS_NVRD = 0x71;       // Request read of node variable
    public static final int CBUS_NENRD = 0x72;      // Request read stored event by index
    public static final int CBUS_RQNPN = 0x73;      // Request read module parameters
    public static final int CBUS_NUMEV = 0x74;      // Number of events stored response
    public static final int CBUS_CANID = 0x75;      // Set canid
                                                    
    public static final int CBUS_EXTC2 = 0x7F;      // Extended opcode with 2 data bytes

    // Opcodes with 4 data bytes
    
    public static final int CBUS_RDCC3 = 0x80;      // 3 byte DCC packet
    public static final int CBUS_WCVO = 0x82;       // Write CV byte Ops mode by handle
    public static final int CBUS_WCVB = 0x83;       // Write CV bit Ops mode by handle
    public static final int CBUS_QCVS = 0x84;       // Read CV
    public static final int CBUS_PCVS = 0x85;       // Report CV
    public static final int CBUS_CABSIG = 0x86;     // Cab signalling    

    public static final int CBUS_ACON = 0x90;       // on event
    public static final int CBUS_ACOF = 0x91;       // off event
    public static final int CBUS_AREQ = 0x92;       // Accessory Request event
    public static final int CBUS_ARON = 0x93;       // Accessory response event on
    public static final int CBUS_AROF = 0x94;       // Accessory response event off
    public static final int CBUS_EVULN = 0x95;      // Unlearn event
    public static final int CBUS_NVSET = 0x96;      // Set a node variable
    public static final int CBUS_NVANS = 0x97;      // Node variable value response
    public static final int CBUS_ASON = 0x98;       // Short event on
    public static final int CBUS_ASOF = 0x99;       // Short event off
    public static final int CBUS_ASRQ = 0x9A;       // Short Request event
    public static final int CBUS_PARAN = 0x9B;      // Single node parameter response
    public static final int CBUS_REVAL = 0x9C;      // Request read of event variable
    public static final int CBUS_ARSON = 0x9D;      // Accessory short response on event
    public static final int CBUS_ARSOF = 0x9E;      // Accessory short response off event
    public static final int CBUS_EXTC3 = 0x9F;      // Extended opcode with 3 data bytes

    // OPcodes with 5 data bytes
    
    public static final int CBUS_RDCC4 = 0xA0;      // 4 byte DCC packet
    public static final int CBUS_WCVS = 0xA2;       // Write CV service mode
                                                    
    public static final int CBUS_ACON1 = 0xB0;      // On event with one data byte
    public static final int CBUS_ACOF1 = 0xB1;      // Off event with one data byte
    public static final int CBUS_REQEV = 0xB2;      // Read event variable in learn mode
    public static final int CBUS_ARON1 = 0xB3;      // Accessory on response (1 data byte)
    public static final int CBUS_AROF1 = 0xB4;      // Accessory off response (1 data byte)
    public static final int CBUS_NEVAL = 0xB5;      // Event variable by index read response
    public static final int CBUS_PNN = 0xB6;        // Response to QNN
    public static final int CBUS_ASON1 = 0xB8;      // Accessory short on with 1 data byte
    public static final int CBUS_ASOF1 = 0xB9;      // Accessory short off with 1 data byte
    public static final int CBUS_ARSON1 = 0xBD;     // Short response event on with one data byte
    public static final int CBUS_ARSOF1 = 0xBE;     // Short response event off with one data byte
    public static final int CBUS_EXTC4 = 0xBF;      // Extended opcode with 4 data bytes

    // Opcodes with 6 data bytes
    
    public static final int CBUS_RDCC5 = 0xC0;      // 5 byte DCC packet
    public static final int CBUS_WCVOA = 0xC1;      // Write CV ops mode by address

    public static final int CBUS_FCLK = 0xCF;       // Fast clock

    public static final int CBUS_ACON2 = 0xD0;      // On event with two data bytes
    public static final int CBUS_ACOF2 = 0xD1;      // Off event with two data bytes
    public static final int CBUS_EVLRN = 0xD2;      // Teach event
    public static final int CBUS_EVANS = 0xD3;      // Event variable read response in learn mode
    public static final int CBUS_ARON2 = 0xD4;      // Accessory on response
    public static final int CBUS_AROF2 = 0xD5;      // Accessory off response

    public static final int CBUS_ASON2 = 0xD8;      // Accessory short on with 2 data bytes
    public static final int CBUS_ASOF2 = 0xD9;      // Accessory short off with 2 data bytes

    public static final int CBUS_ARSON2 = 0xDD;     // Short response event on with two data bytes
    public static final int CBUS_ARSOF2 = 0xDE;     // Short response event off with two data bytes
    public static final int CBUS_EXTC5 = 0xDF;      // Extended opcode with 5 data bytes

    // Opcodes with 7 data bytes
    
    public static final int CBUS_RDCC6 = 0xE0;      // 6 byte DCC packets
    public static final int CBUS_PLOC = 0xE1;       // Loco session report
    public static final int CBUS_NAME = 0xE2;       // Module name response
    public static final int CBUS_STAT = 0xE3;       // Command station status report

    public static final int CBUS_PARAMS = 0xEF;     // Node parameters response

    public static final int CBUS_ACON3 = 0xF0;      // On event with 3 data bytes
    public static final int CBUS_ACOF3 = 0xF1;      // Off event with 3 data bytes
    public static final int CBUS_ENRSP = 0xF2;      // Read node events response
    public static final int CBUS_ARON3 = 0xF3;      // Accessory on response
    public static final int CBUS_AROF3 = 0xF4;      // Accessory off response
    public static final int CBUS_EVLRNI = 0xF5;     // Teach event using event indexing
    public static final int CBUS_ACDAT = 0xF6;      // Accessory data event: 5 bytes of node data (eg: 
    public static final int CBUS_ARDAT = 0xF7;      // Accessory data response
    public static final int CBUS_ASON3 = 0xF8;      // Accessory short on with 3 data bytes
    public static final int CBUS_ASOF3 = 0xF9;      // Accessory short off with 3 data bytes
    public static final int CBUS_DDES = 0xFA;       // Short data frame aka device data event (device i
    public static final int CBUS_DDRS = 0xFB;       // Short data frame response aka device data respon
                                                    
    public static final int CBUS_ARSON3 = 0xFD;     // Short response event on with 3 data bytes
    public static final int CBUS_ARSOF3 = 0xFE;     // Short response event off with 3 data bytes
    public static final int CBUS_EXTC6 = 0xFF;      // Extended opcode with 6 data byes

    /**
     * Programming modes
     */
    public static final int CBUS_PROG_DIRECT_BYTE = 0;
    public static final int CBUS_PROG_DIRECT_BIT = 1;
    public static final int CBUS_PROG_PAGED = 2;
    public static final int CBUS_PROG_REGISTER = 3;
    public static final int CBUS_PROG_ADDRESS = 4;
    public static final int CBUS_OPS_BYTE = 5;

    /**
     * Error codes returned by CBUS_ERR
     */
    public static final int CBUS_DCC_ERR_LOCO_STACK_FULL = 1;
    public static final int CBUS_DCC_ERR_LOCO_ADDRESS_TAKEN = 2;
    public static final int CBUS_DCC_ERR_SESSION_NOT_PRESENT = 3;
    public static final int CBUS_DCC_ERR_CONSIST_EMPTY = 4;
    public static final int CBUS_DCC_ERR_LOCO_NOT_FOUND = 5;
    public static final int CBUS_DCC_ERR_CAN_BUS_ERROR = 6;
    public static final int CBUS_DCC_ERR_INVALID_REQUEST = 7;
    public static final int CBUS_DCC_ERR_SESSION_CANCELLED = 8;

    // Error codes for OPC_CMDERR

    public static final int CBUS_ERR_INV_CMD         = 1;
    public static final int CBUS_ERR_NOT_LRN         = 2;
    public static final int CBUS_ERR_NOT_SETUP       = 3;
    public static final int CBUS_ERR_TOO_MANY_EVENTS = 4;
    public static final int CBUS_ERR_NO_EV           = 5;  
    public static final int CBUS_ERR_INV_EV_IDX      = 6;
    public static final int CBUS_ERR_INVALID_EVENT   = 7;
    //CBUS_ERR_INV_EN_IDX     = 8;    now reserved
    public static final int CBUS_ERR_INV_PARAM_IDX   = 9;
    public static final int CBUS_ERR_INV_NV_IDX      = 10;
    public static final int CBUS_ERR_INV_EV_VALUE    = 11;
    public static final int CBUS_ERR_INV_NV_VALUE    = 12;
    
    /**
     * Status codes for OPC_SSTAT
     */
    public static final int SSTAT_NO_ACK = 1;
    public static final int SSTAT_OVLD = 2;
    public static final int SSTAT_WR_ACK = 3;
    public static final int SSTAT_BUSY = 4;
    public static final int SSTAT_CV_ERROR = 5;

    /**
     * Event types
     */
    public static final int EVENT_ON = 0;
    public static final int EVENT_OFF = 1;
    public static final int EVENT_EITHER = 2;

    /**
     * CBUS Priorities
     */
    public static final int DEFAULT_DYNAMIC_PRIORITY = 2;
    public static final int DEFAULT_MINOR_PRIORITY = 3;

        /**
     * Function bits for group1
     */
    public static final int CBUS_F0 = 16;
    public static final int CBUS_F1 = 1;
    public static final int CBUS_F2 = 2;
    public static final int CBUS_F3 = 4;
    public static final int CBUS_F4 = 8;

    /**
     * Function bits for group2
     */
    public static final int CBUS_F5 = 1;
    public static final int CBUS_F6 = 2;
    public static final int CBUS_F7 = 4;
    public static final int CBUS_F8 = 8;

    /**
     * Function bits for group3
     */
    public static final int CBUS_F9 = 1;
    public static final int CBUS_F10 = 2;
    public static final int CBUS_F11 = 4;
    public static final int CBUS_F12 = 8;

    /**
     * Function bits for group4
     */
    public static final int CBUS_F13 = 1;
    public static final int CBUS_F14 = 2;
    public static final int CBUS_F15 = 4;
    public static final int CBUS_F16 = 8;
    public static final int CBUS_F17 = 0x10;
    public static final int CBUS_F18 = 0x20;
    public static final int CBUS_F19 = 0x40;
    public static final int CBUS_F20 = 0x80;

    /**
     * Function bits for group5
     */
    public static final int CBUS_F21 = 1;
    public static final int CBUS_F22 = 2;
    public static final int CBUS_F23 = 4;
    public static final int CBUS_F24 = 8;
    public static final int CBUS_F25 = 0x10;
    public static final int CBUS_F26 = 0x20;
    public static final int CBUS_F27 = 0x40;
    public static final int CBUS_F28 = 0x80;

    /**
     * Throttle modes
     */
    public static final int CBUS_SS_128 = 0;
    public static final int CBUS_SS_14 = 1;
    public static final int CBUS_SS_28_INTERLEAVE = 2;
    public static final int CBUS_SS_28 = 3;

    // Aspect codes for OPC_CABSIG

    // First aspect byte

    public static final int CBUS_SASP_DANGER         = 0;
    public static final int CBUS_SASP_CAUTION        = 1;
    public static final int CBUS_SASP_PRELIM_CAUTION = 2;
    public static final int CBUS_SASP_PROCEED        = 3;
    public static final int CBUS_SASP_CALLON         = 4;    // Set bit 2 for call-on - main aspect will usually be at danger
    public static final int CBUS_SASP_THEATRE        = 8;    // Set bit 3 to 0 for upper nibble is feather lcoation, set 1 for upper nibble is theatre code

    // Second Aspect byte

    public static final int CBUS_SASP_LIT            = 0;    // Set bit 0 to indicate lit
    public static final int CBUS_SASP_LUNAR          = 1;    // Set bit 1 for lunar indication

    // Remaining bits in second aspect byte yet to be defined - use for other signalling systems	


    // Parameter index numbers (readable by OPC_RQNPN, returned in OPC_PARAN)
    // Index numbers count from 1, subtract 1 for offset into parameter block
    // Note that RQNPN with index 0 returns the parameter count

    public static final int CBUS_PAR_MANU       = 1;    // Manufacturer id
    public static final int CBUS_PAR_MINVER     = 2;    // Minor version letter
    public static final int CBUS_PAR_MTYP       = 3;    // Module type code
    public static final int CBUS_PAR_EVTNUM     = 4;    // Number of events supported
    public static final int CBUS_PAR_EVNUM      = 5;    // Event variables per event
    public static final int CBUS_PAR_NVNUM      = 6;    // Number of Node variables
    public static final int CBUS_PAR_MAJVER     = 7;    // Major version number
    public static final int CBUS_PAR_FLAGS      = 8;    // Node flags
    public static final int CBUS_PAR_CPUID      = 9;    // Processor type
    public static final int CBUS_PAR_BUSTYPE    = 10;   // Bus type
    public static final int CBUS_PAR_LOAD       = 11;   // load address, 4 bytes
    public static final int CBUS_PAR_CPUMID     = 15;   // CPU manufacturer's id as read from the chip config space, 4 bytes (note - read from cpu at runtime, so not included in checksum)
    public static final int CBUS_PAR_CPUMAN     = 19;   // CPU manufacturer code
    public static final int CBUS_PAR_BETA       = 20;   // Beta revision (numeric), or 0 if release

    // Offsets to other values stored at the top of the parameter block.
    // These are not returned by opcode public static final int CBUS_PARAN, but are present in the hex
    // file for FCU.

    public static final int CBUS_PAR_COUNT = 0x18;    // Number of parameters implemented
    public static final int CBUS_PAR_NAME  = 0x1A;    // 4 byte Address of Module type name, up to 8 characters null terminated
    public static final int CBUS_PAR_CKSUM = 0x1E;    // Checksum word at end of parameters

    // Flags in PAR_FLAGS

    public static final int CBUS_PF_NOEVENTS =   0;
    public static final int CBUS_PF_CONSUMER =   1;
    public static final int CBUS_PF_PRODUCER =   2;
    public static final int CBUS_PF_COMBI    =   3;
    public static final int CBUS_PF_FLiM     =   4;
    public static final int CBUS_PF_BOOT     =   8;
    public static final int CBUS_PF_COE	     =   16;    // Module can consume its own events

    // BUS type that module is connected to

    public static final int CBUS_PB_CAN = 1;
    public static final int CBUS_PB_ETH = 2;
    public static final int CBUS_PB_MIW = 3;

    // Processor manufacturer codes

    public static final int CPUM_MICROCHIP =  01;
    public static final int CPUM_ATMEL     =  02;
    public static final int CPUM_ARM       =  03;

    // Processor type codes (identifies to FCU for bootload compatiblity)

    public static final int P18F2480       = 1;
    public static final int P18F4480       = 2;
    public static final int P18F2580       = 3;
    public static final int P18F4580       = 4;
    public static final int P18F2585       = 5;
    public static final int P18F4585       = 6;
    public static final int P18F2680       = 7;
    public static final int P18F4680       = 8;
    public static final int P18F2682       = 9;
    public static final int P18F4682       = 10;
    public static final int P18F2685       = 11;
    public static final int P18F4685       = 12;

    public static final int P18F25K80      = 13;
    public static final int P18F45K80      = 14;
    public static final int P18F26K80      = 15;
    public static final int P18F46K80      = 16;
    public static final int P18F65K80      = 17;
    public static final int P18F66K80      = 18;

    public static final int P32MX534F064   = 30;
    public static final int P32MX564F064   = 31;
    public static final int P32MX564F128   = 32;
    public static final int P32MX575F256   = 33;
    public static final int P32MX575F512   = 34;
    public static final int P32MX764F128   = 35;
    public static final int P32MX775F256   = 36;
    public static final int P32MX775F512   = 37;
    public static final int P32MX795F512   = 38;


    // ARM Processor type codes (identifies to FCU for bootload compatiblity)

    public static final int ARM1176JZ_FS   = 1;   // As used in Raspberry Pi
    public static final int ARMCortex_A7   = 2;   // As Used in Raspberry Pi 2
    public static final int ARMCortex_A53  = 3;   // As used in Raspberry Pi 3
    
}
