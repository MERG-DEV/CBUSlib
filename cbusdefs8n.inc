
; CBUS definitions for use in PIC assembler

; (c) Pete Brownlow  2011-2016 software@upsys.co.uk

; NOTE: To display this file with everything correctly lined up set your editor or IDE tab stop to 4  

; This work is licensed under the:
;      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
;       http://creativecommons.org/licenses/by-nc-sa/4.0/

; See the above link for full details

;  For commercial use, please contact the original copyright holder(s) to agree licensing terms
    
    
; These definitions are taken from CBUS specifications document rev 8d embedded in developer guide 6b.


; Pete Brownlow 6/4/11 - Original from spec version 7e 
; Pete Brownlow 7/6/11 - Update to spec version 7f, add new module type codes
; Pete Brownlow 4/7/11 - Update to spec version 7g.
; Pete Brownlow 14/8/11 - Updated to spec ver 7h
; Pete Brownlow 20/2/12 - Updated to spec ver 8a, Rocrail and animated modeller module types added
; Pete Brownlow 10/4/12 - Updated to spec ver 8b, extended parameter bytes
; Roger Healey	13/7/12 - Add CANTOTI
; Pete Brownlow 15/9/12 - Updated to spec ver 8c, added 0x5D ENUM and 0x75 CANID
; Roger Healey  15/2/13 - Now at version d. Added new Module Ids (as per ModuleIds Issue 9) and  
;			- align Processor Ids with constants.inc
;			- and added .(fullstop) before each processor Id > 9
;			- and added OPC_NNRST
; Roger Healey	27/04/13 - Added CANSIG8 and CANSIG64
; Roger Healey  06/08/13 - add CANCOND8C
; Roger Healey 22/01/14 = add CANPAN, CANACE3C, CANPanel and CANMIO
; Phil Wheeler  22/1/14 - Corrected some typos
; Pete Brownlow 22/1/14 - Added NN_RSM, fix CANGC1e type definition, add parameter definitions for manufacturer's CPU id
; Phil Wheeler	 1/2/14	- Added PRM_BETA, dotted some more decimal numbers
; Phil Wheeler	 9/3/14 - Corrected CANSIG MTYP definitions
; Pete Brownlow 19/4/14 - Added module type defs for CANTOTIMIO, CANBIP
; Pete Brownlow 5/5/14  - Remove canacc5c and cantotimio, add cansol
; Pete Brownlow 06/7/15 - Ver 8j Add new module types as per modules definitions v17 : CANCDU, CANACC4CDU, CANWiBase, WiCAB, CANWiFi, CANFTT
;                                Add new opcode ALOC, added CPU manufacturer code CPUM_ATMEL
; Mike Bolton	 2/3/16	- Ver 8k    Add module type CANRFID8
; Pete Brownlow  2/7/16 - Ver 8m     Add new module types CANHNDST, CANTCHNDST, CANmchRFID and CANPiWi  
;                                    Add processor codes for ARM chips used in Raspberry Pi's  
; Pete Brownlow	 16/2/17 - Ver 8n   Add module ids to bring it up to module ids document ver 25   


; Manufacturers

MANU_MERG	equ	.165	; http:;www.merg.co.uk
MANU_ROCRAIL	equ	.70		; http:;www.rocrail.net
MANU_SPECTRUM	equ	.80		; http:;animatedmodeler.com  (Spectrum Engineering)

; MERG Module types

MTYP_SLIM       equ .0  ; default type code for SLiM nodes
MTYP_CANACC4    equ .1	; Solenoid point driver
MTYP_CANACC5    equ .2	; Motorised point driver
MTYP_CANACC8    equ .3	; 8 digital outputs
MTYP_CANACE3    equ .4	; Control panel switch/button encoder
MTYP_CANACE8C   equ .5	; 8 digital inputs
MTYP_CANLED     equ .6	; 64 led driver
MTYP_CANLED64   equ .7	; 64 led driver (multi leds per event)
MTYP_CANACC4_2  equ .8	; 12v version of CANACC4  
MTYP_CANCAB     equ .9	; CANCAB hand throttle   
MTYP_CANCMD     equ .10	; CANCMD command station
MTYP_CANSERVO	equ .11	; 8 servo driver (on canacc8 or similar hardware)
MTYP_CANBC      equ .12	; BC1a command station
MTYP_CANRPI     equ .13	; RPI and RFID interface
MTYP_CANTTCA	equ .14	; Turntable controller (turntable end)
MTYP_CANTTCB	equ .15 ; Turntable controller (control panel end)
MTYP_CANHS      equ .16	; Handset controller for old BC1a type handsets
MTYP_CANTOTI	equ .17	; Track occupancy detector

MTYP_CAN8I8O	equ .18	;8 input/8 output
MTYP_CANSERVO8C	equ .19	;Combi version of CANSERVO
MTYP_CANRFID	equ .20	;RFID module
MTYP_CANTC4     equ .21
MTYP_CANACE16C	equ .22	;16 input version of CANACE8C
MTYP_CANIO8     equ .23
MTYP_CANSNDx	equ .24
MTYP_CANETHER	equ .25	;Ethernet interface module
MTYP_CANSIG64	equ .26	;CANSIG firmware for CANLED64 hardware
MTYP_CANSIG8	equ .27	;CANSIG firmware for CANACC8 hardware
MTYP_CANCOND8C	equ .28	;CAN conditional module
MTYP_CANPAN	equ .29	;Combines 32 switches and 32 LEDs
MTYP_CANACE3C	equ .30	;Enhanced version of CANACE3
MTYP_CANPanel	equ .31	;Combines 64 switches with 64 LEDs
MTYP_CANMIO	equ .32	;Multiple IO module
MTYP_CANACE8MIO	equ .33	;Multiple IO module emulating ACE8C
MTYP_CANSOL     equ .34 ;Solenoid driver module
MTYP_CANBIP     equ .35 ;Bipolar IO module with additional 8 I/O pins
MTYP_CANCDU     equ .36 ;Solenoid driver module with additional 6 I/O pins
MTYP_CANACC4CDU equ .37 ;CANACC4 firmware modified for CANCDU
MTYP_CANWiBase  equ .38 ;CAN to MiWi base station
MTYP_WiCAB      equ .39 ;Wireless cab using MiWi protocol
MTYP_CANWiFi    equ .40 ;CAN to WiFi connection with Withrottle to CBUS protocol conversion
MTYP_CANFTT     equ .41 ; Turntable controller configured using FLiM
MTYP_CANHNDST   equ .42 ; Handset (alternative to CANCAB)
MTYP_CANTCHNDST equ .43 ; Touchscreen handset
MTYP_CANRFID8   equ .44 ; multi-channel RFID reader
MTYP_CANmchRFID equ .45 ; either a 2ch or 8ch RFID reader
MTYP_CANPiWi    equ .46 ; a Raspberry Pi based module for WiFi  
MTYP_CAN4DC	equ .47 ; DC train controller
MTYP_CANELEV	equ .48	; Nelevator controller
MTYP_CANSCAN	equ .49	; 128 switch inputs
MTYP_CANMIO_SVO	equ .50	; 16MHz 25k80 version of CANSERVO8c
MTYP_CANMIO_INP	equ .51	; 16MHz 25k80 version of CANACE8MIO
MTYP_CANMIO_OUT	equ .52	; 16MHz 25k80 version of CANACC8
MTYP_CANBIP_OUT	equ .53	; 16MHz 25k80 version of CANACC5
MTYP_CANASTOP	equ .54	; DCC stop generator    

MTYP_CAN_SW     equ 0xFF ; Software nodes
MTYP_EMPTY      equ 0xFE ; Empty module, bootloader only
MTYP_CANUSB     equ 0xFD ; Reserved for future use


; Rocrail Module types

MTYP_CANGC1		equ	 .1	; RS232 PC interface
MTYP_CANGC2		equ	 .2	; 16 I/O
MTYP_CANGC3		equ	 .3	; Command station (derived from cancmd)
MTYP_CANGC4		equ	 .4	; 8 channel RFID reader
MTYP_CANGC5		equ	 .5	; Cab for fixed panels (derived from cancab)
MTYP_CANGC6		equ	 .6	; 4 channel servo controller
MTYP_CANGC7		equ	 .7	; Fast clock module
MTYP_CANGC1e		equ	 .11	; CAN<->Ethernet interface


; Spectrum Engineering Animated Modeller module types

MTYP_AMCTRLR	equ  .1	; Animation controller (firmware derived from cancmd)
MTYP_DUALCAB	equ  .2	; Dual cab based on cancab


;  CBUS opcodes list 

; Packets with no data bytes

OPC_ACK     EQU 0x00    ; General ack
OPC_NAK     EQU 0x01    ; General nak
OPC_HLT     EQU	0x02    ; Bus Halt
OPC_BON     EQU	0x03    ; Bus on
OPC_TOF     EQU	0x04    ; Track off
OPC_TON     EQU	0x05    ; Track on
OPC_ESTOP   EQU	0x06    ; Track stopped
OPC_ARST    EQU	0x07    ; System reset
OPC_RTOF    EQU	0x08    ; Request track off
OPC_RTON    EQU	0x09    ; Request track on
OPC_RESTP   EQU	0x0a    ; Request emergency stop all
OPC_RSTAT   EQU 0x0c    ; Request node status
OPC_QNN     EQU 0x0d    ; Query nodes

OPC_RQNP    EQU 0x10    ; Read node parameters
OPC_RQMN    EQU 0x11	; Request name of module type

; Packets with 1 data byte

OPC_KLOC    EQU 0x21    ; Release engine by handle
OPC_QLOC    EQU	0x22    ; Query engine by handle
OPC_DKEEP   EQU 0x23	; Keep alive for cab

OPC_DBG1    EQU 0x30    ; Debug message with 1 status byte
OPC_EXTC    EQU 0x3F    ; Extended opcode

; Packets with 2 data bytes

OPC_RLOC    EQU 0x40    ; Request session for loco
OPC_QCON    EQU 0x41	; Query consist
OPC_SNN     EQU	0x42    ; Set node number
OPC_ALOC    EQU 0X43    ; Allocate loco (used to allocate to a shuttle in cancmd)
OPC_STMOD   EQU	0x44    ; Set Throttle mode
OPC_PCON    EQU	0x45    ; Consist loco
OPC_KCON    EQU	0x46    ; De-consist loco
OPC_DSPD    EQU	0x47    ; Loco speed/dir
OPC_DFLG    EQU 0x48    ; Set engine flags
OPC_DFNON   EQU 0x49	; Loco function on
OPC_DFNOF   EQU 0x4A	; Loco function off
OPC_SSTAT   EQU	0x4C    ; Service mode status
OPC_NNRSM   EQU 0x4F    ; Reset to manufacturer's defaults

OPC_RQNN    EQU	0x50    ; Request Node number in setup mode
OPC_NNREL   EQU	0x51    ; Node number release
OPC_NNACK   EQU	0x52    ; Node number acknowledge
OPC_NNLRN   EQU	0x53    ; Set learn mode
OPC_NNULN   EQU	0x54    ; Release learn mode
OPC_NNCLR   EQU	0x55    ; Clear all events
OPC_NNEVN   EQU	0x56    ; Read available event slots
OPC_NERD    EQU 0x57    ; Read all stored events
OPC_RQEVN   EQU 0x58    ; Read number of stored events
OPC_WRACK   EQU 0x59	; Write acknowledge
OPC_RQDAT   EQU	0x5A	; Request node data event
OPC_RQDDS   EQU 0x5B    ; Request short data frame
OPC_BOOT    EQU	0x5C    ; Put node into boot mode
OPC_ENUM    EQU	0x5D	; Force canid self enumeration
OPC_NNRST   EQU 0x5E	; Reset node
OPC_EXTC1   EQU 0x5F    ; Extended opcode with 1 data byte

; Packets with 3 data bytes

OPC_DFUN    EQU	0x60    ; Set engine functions
OPC_GLOC    EQU 0x61	; Get loco (with support for steal/share)
OPC_ERR     EQU	0x63    ; Command station error
OPC_CMDERR  EQU 0x6F    ; Errors from nodes during config

OPC_EVNLF   EQU	0x70    ; Event slots left response
OPC_NVRD    EQU	0x71    ; Request read of node variable
OPC_NENRD   EQU 0x72    ; Request read stored event by index
OPC_RQNPN   EQU	0x73	; Request read module parameters
OPC_NUMEV   EQU 0x74    ; Number of events stored response
OPC_CANID   EQU 0x75	; Set canid
OPC_EXTC2   EQU 0x7F    ; Extended opcode with 2 data bytes

; Packets with 4 data bytes

OPC_RDCC3   EQU	0x80    ; 3 byte DCC packet
OPC_WCVO    EQU	0x82    ; Write CV byte Ops mode by handle
OPC_WCVB    EQU	0x83    ; Write CV bit Ops mode by handle
OPC_QCVS    EQU	0x84    ; Read CV
OPC_PCVS    EQU	0x85    ; Report CV

OPC_ACON    EQU	0x90	; on event
OPC_ACOF    EQU	0x91	; off event
OPC_AREQ    EQU	0x92    ; Accessory request event
OPC_ARON    EQU 0x93    ; Accessory response event on
OPC_AROF    EQU 0x94    ; Accessory response event off
OPC_EVULN   EQU	0x95    ; Unlearn event
OPC_NVSET   EQU	0x96    ; Set a node variable
OPC_NVANS   EQU	0x97    ; Node variable value response
OPC_ASON    EQU	0x98    ; Short event on
OPC_ASOF    EQU	0x99    ; Short event off
OPC_ASRQ    EQU	0x9A    ; Short request event
OPC_PARAN   EQU 0x9B    ; Single node parameter response
OPC_REVAL   EQU 0x9C    ; Request read of event variable
OPC_ARSON   EQU	0x9D	; Accessory short response on event
OPC_ARSOF   EQU	0x9E	; Accessory short response off event
OPC_EXTC3   EQU 0x9F    ; Extended opcode with 3 data bytes

; Packets with 5 data bytes

OPC_RDCC4   EQU	0xA0    ; 4 byte DCC packet
OPC_WCVS    EQU	0xA2    ; Write CV service mode

OPC_ACON1   EQU 0xB0    ; On event with one data byte
OPC_ACOF1   EQU 0xB1    ; Off event with one data byte
OPC_REQEV   EQU	0xB2    ; Read event variable in learn mode
OPC_ARON1   EQU	0xB3	; Accessory on response (1 data byte)
OPC_AROF1   EQU	0xB4    ; Accessory off response (1 data byte)
OPC_NEVAL   EQU 0xB5    ; Event variable by index read response
OPC_PNN	    EQU	0xB6	; Response to QNN
OPC_ASON1   EQU	0xB8	; Accessory short on with 1 data byte
OPC_ASOF1   EQU	0xB9	; Accessory short off with 1 data byte
OPC_ARSON1  EQU	0xBD	; Short response event on with one data byte
OPC_ARSOF1  EQU	0xBE	; Short response event off with one data byte
OPC_EXTC4   EQU 0xBF    ; Extended opcode with 4 data bytes

; Packets with 6 data bytes

OPC_RDCC5   EQU	0xC0    ; 5 byte DCC packet
OPC_WCVOA   EQU	0xC1    ; Write CV ops mode by address
OPC_FCLK    EQU	0xCF	; Fast clock

OPC_ACON2   EQU 0xD0    ; On event with two data bytes
OPC_ACOF2   EQU 0xD1    ; Off event with two data bytes
OPC_EVLRN   EQU	0xd2    ; Teach event
OPC_EVANS   EQU	0xd3    ; Event variable read response in learn mode
OPC_ARON2   EQU 0xD4    ; Accessory on response 
OPC_AROF2   EQU 0xD5    ; Accessory off response
OPC_ASON2   EQU	0xD8	; Accessory short on with 2 data bytes
OPC_ASOF2   EQU	0xD9	; Accessory short off with 2 data bytes
OPC_ARSON2  EQU	0xDD	; Short response event on with two data bytes
OPC_ARSOF2  EQU	0xDE	; Short response event off with two data bytes
OPC_EXTC5   EQU 0xDF    ; Extended opcode with 5 data bytes

; Packets with 7 data bytes

OPC_RDCC6   EQU	0xE0    ; 6 byte DCC packets
OPC_PLOC    EQU	0xE1    ; Loco session report
OPC_NAME    EQU 0xE2    ; Module name response
OPC_STAT    EQU 0xE3    ; Command station status report
OPC_PARAMS  EQU	0xEF    ; Node parameters response

OPC_ACON3   EQU 0xF0    ; On event with 3 data bytes
OPC_ACOF3   EQU 0xF1    ; Off event with 3 data bytes
OPC_ENRSP   EQU	0xF2    ; Read node events response
OPC_ARON3   EQU 0xF3    ; Accessory on response 
OPC_AROF3   EQU 0xF4    ; Accessory off response
OPC_EVLRNI  EQU	0xF5    ; Teach event using event indexing
OPC_ACDAT   EQU	0xF6    ; Accessory data event: 5 bytes of node data (eg: RFID)
OPC_ARDAT   EQU	0xF7	; Accessory data response
OPC_ASON3   EQU	0xF8	; Accessory short on with 3 data bytes
OPC_ASOF3   EQU	0xF9	; Accessory short off with 3 data bytes
OPC_DDES    EQU	0xFA	; Short data frame aka device data event (device id plus 5 data bytes)
OPC_DDRS    EQU	0xFB	; Short data frame response aka device data response
OPC_ARSON3  EQU	0xFD	; Short response event on with 3 data bytes
OPC_ARSOF3  EQU	0xFE	; Short response event off with 3 data bytes
OPC_EXTC6   EQU	0xFF	; Extended opcode with 6 data byes


; Modes for STMOD

TMOD_SPD_MASK   		EQU	.3
TMOD_SPD_128    		EQU	.0
TMOD_SPD_14     		EQU	.1
TMOD_SPD_28I    		EQU	.2
TMOD_SPD_28     		EQU	.3

; Error codes for OPC_ERR

ERR_LOCO_STACK_FULL     EQU	.1
ERR_LOCO_ADDR_TAKEN     EQU	.2
ERR_SESSION_NOT_PRESENT EQU	.3
ERR_CONSIST_EMPTY	EQU	.4
ERR_LOCO_NOT_FOUND    	EQU	.5
ERR_CMD_RX_BUF_OFLOW	EQU	.6
ERR_INVALID_REQUEST	EQU	.7
ERR_SESSION_CANCELLED	EQU	.8

; Status codes for OPC_SSTAT

SSTAT_NO_ACK    		EQU	.1
SSTAT_OVLD      		EQU	.2
SSTAT_WR_ACK    		EQU	.3
SSTAT_BUSY      		EQU	.4
SSTAT_CV_ERROR  		EQU	.5

; Error codes for OPC_CMDERR

CMDERR_INV_CMD			EQU	.1
CMDERR_NOT_LRN			EQU	.2
CMDERR_NOT_SETUP		EQU	.3
CMDERR_TOO_MANY_EVENTS		EQU	.4
; CMDERR_NO_EV			EQU	.5		NOW RESERVED
CMDERR_INV_EV_IDX		EQU	.6
CMDERR_INVALID_EVENT		EQU	.7
    ; CMDERR_INV_EN_IDX		EQU	.8		NOW RESERVED
CMDERR_INV_PARAM_IDX		EQU	.9
CMDERR_INV_NV_IDX		EQU	.10
CMDERR_INV_EV_VALUE		EQU	.11
CMDERR_INV_NV_VALUE		EQU	.12

; Parameter index numbers (readable by OPC_RQNPN, returned in OPC_PARAN)
; Index numbers count from 1, subtract 1 for offset into parameter block
; Note that RQNPN with index 0 returns the parameter count

PAR_MANU		EQU .1	; Manufacturer id
PAR_MINVER		EQU .2	; Minor version letter
PAR_MTYP		EQU .3	; Module type code				
PAR_EVTNUM		EQU .4	; Number of events supported
PAR_EVNUM		EQU .5	; Event variables per event
PAR_NVNUM		EQU .6	; Number of Node variables
PAR_MAJVER		EQU .7	; Major version number
PAR_FLAGS		EQU .8	; Node flags
PAR_CPUID		EQU .9	; Processor type
PAR_BUSTYPE		EQU .10	; Bus type
PAR_LOAD		EQU .11	; load address, 4 bytes
PAR_CPUMID              EQU .15 ; CPU manufacturer's id as read from the chip config space, 4 bytes (note - read from cpu at runtime, so not included in checksum)
PAR_CPUMAN              EQU .19 ; CPU manufacturer code
PAR_BETA		EQU .20	; Beta revision (numeric), or 0 if release

; Offsets to other values stored at the top of the parameter block.
; These are not returned by opcode PARAN, but are present in the hex 
; file for FCU. 

PAR_COUNT		EQU	0x18 ; Number of parameters implemented
PAR_NAME		EQU	0x1A ; 4 byte Address of Module type name, up to 8 characters null terminated
PAR_CKSUM		EQU	0x1E ; Checksum word at end of parameters

; Flags in PAR_FLAGS

PF_NOEVENTS		equ	.0
PF_CONSUMER		equ	.1
PF_PRODUCER		equ	.2
PF_COMBI		equ	.3
PF_FLiM			equ	.4
PF_BOOT			equ	.8


; BUS type that module is connected to

PB_CAN			equ .1
PB_ETH			equ .2
PB_MIWI                 equ .3

; Processor manufacturer codes

CPUM_MICROCHIP          equ	.1
CPUM_ATMEL              equ	.2
CPU_ARM                 equ	.3
		 
; Microchip Processor type codes (identifies to FCU for bootload compatiblity)

P18F2480		equ	.1
P18F4480		equ	.2
P18F2580		equ	.3
P18F4580		equ	.4
P18F2585		equ	.5
P18F4585		equ	.6
P18F2680		equ	.7
P18F4680		equ	.8
P18F2682		equ	.9
P18F4682		equ	.10
P18F2685		equ	.11
P18F4685		equ	.12
; Version d changed the following 6 to align with constants. inc and FCU usage
P18F25K80		equ	.13
P18F45K80		equ	.14
P18F26K80		equ	.15
P18F46K80		equ	.16
P18F65K80		equ	.17
P18F66K80		equ	.18

P32MX534F064		equ	.30
P32MX564F064		equ	.31
P32MX564F128		equ	.32
P32MX575F256		equ	.33
P32MX575F512		equ	.34
P32MX764F128		equ	.35
P32MX775F256		equ	.36
P32MX775F512		equ	.37
P32MX795F512		equ	.38

; ARM Processor type codes (identifies to FCU for bootload compatiblity)

ARM1176JZF_S		equ	.1   ; As used in Raspberry Pi
ARMCortex_A7            equ	.2   ; As Used in Raspberry Pi 2
ARMCortex_A53           equ	.3   ; As used in Raspberry Pi 3