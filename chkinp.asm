; filename chkinp.asm
;
; Common routine checks for changes on the additional inputs when used in CANMIO,
; CANBIP or CANCDU hardware. These inputs can also be used in orginal MERG hardware
; if input signals are wired to where the SLiM switches are normally fitted.
;
; The inputs are only checked when in FLiM, a CBUS event is sent for a change in the input state.
;
; The CANMIO range SLiM routines are also in this source file
;
; Because the firmware is built in absolute mode and uses CBLOCK to define
; the RAM usage, this file is included rather than linked
;
; Search the main firmwware source file for "additional inputs" to find references to this source module
;
; Original Rev A coded  01/02/14 PNB

#define INP9    PORTB,0     ; These inputs are numbered 9 to 15, to be separate from the main I/O being 1 to 8
#define INP10   PORTB,1
#define INP11   PORTB,4
#define INP12   PORTB,5
#define INP13   PORTA,1

        ifdef   CE_BIT
          ifdef CANCDU
#define INP14   PORTA,0
          else  ;CANACC4
#define INP14   PORTA,5
          endif
        else    ; All other modules

#define INP14   PORTA,0
#define INP15   PORTA,3
#define INP16   PORTA,5
        endif

ipidxfirs   equ 8               ; Index of the first additional input, counting from 0
ipidxlast   equ .15             ; Index of the last additional input, counting from 0
dbloops     equ 20              ; Debounce count - number of times round main loop inputs must be stable before event sent

; **** Subroutine called during setup to intialise the inputs status ****

inpset  call    getinps             ; Get current status
        movwf   inputs              ; Save as curent status
        movwf   dbinputs            ; Set currently debouncing as the same (ie: no change)
        clrf    dbcount             ; no debounce in progress
        return

; **** Check inputs for any changes ****
; Sends any change events that need to be sent

chkinp  btfss	Mode,1			; Are we in FLiM?
		bra		inpdun          ; no, so skip inputs check

; ??? for canace8mio only, check startup timer
;;	In startup period?
;		movf	SUtimer,W	;get startup timer
;		bnz		end_scan	;Still starting, ignore
;


        call    getinps         ; Update status of inputs- returns with new status in w and stored in rawinps

; Has anything changed?

        subwf   inputs,w
        bnz     changed
        movff   inputs,dbinputs     ; Set debouncing values to the same as the stored values
        bra     inpdun

changed movf    rawinps,w
        subwf   dbinputs,w
        bz      debouncing
        movff   rawinps,dbinputs    ; Set value we are debouncing against
        movlw   dbloops            ; Start debounce counter
        movwf   dbcount
        bra     inpdun

debouncing
        decf    dbcount             ; inputs stable long enough?
        bnz     inpdun              ; No - continue to main loop

;      Work out what event(s) to send

        movf    dbinputs,w
        xorwf   inputs,w          ; w now contains a 1 bit for each input we need to send an event for, rawinps contains the input statuses
        movff   dbinputs,inputs   ; update stataus of inputs
        call    sndinpevs         ; send the events
 

inpdun  return

; **** Called to output SOD events for additional inputs ***************
; Called by CANACE8C and CANSERVO but bypassed by CANACC8 and CANACC5 which use timer interrupt for SOD events

xtrasod movlw   ipidxfirs        ; input number
        movwf   Op_fb
        clrf    EVtemp
        comf    EVtemp          ; set to 0xFF to flag do all inputs

nxsod   call    inpsod          ; Do sod for next additional input
        bz      sodone          ; inpsod sets zero flag after last one
        incf    Op_fb
    	movlw	.200            ; Outer loop delay counter
		movwf	Count1
        call    dely2           ; Do delay (note: cannot use sdely because it is not present in all module types)
        bra     nxsod

sodone  return


; ****  Called for each input on start of day, input number to send is in Op_fb ****
;  evtemp is set to 0xFF when SOD for all feedbacks, any other value is for a single feedback to do not do SOD

inpsod
        movf    EVtemp,w
        sublw   0xFF                ; Check if SOD required
        bnz     nxtsod              ; Skip this routine if not
        movff   Op_fb,dbcount       ; Get input number
        movff   inputs,rawinps      ; Status to send
        movlw   0x01
        movwf   dbinputs             ; Will be used create bit mask for which input
        movlw   ipidxfirs-1
        subwf   dbcount,F           ; Subtract offset so we count from 0
fndinp  dcfsnz  dbcount
        bra     sendsod
        rlncf   dbinputs            ; Shift bit into correct position for this input
        bra     fndinp

sendsod movf    dbinputs,w
        call    sndinpevs           ; Transmit start of day event
nxtsod  movlw   ipidxlast
        subwf   Op_fb,w             ; Set zero flag if last one
        return

; **** Read in the inputs, sets input map into rawinps and returns with rawinps value in w ****

getinps clrf    rawinps
        btfss   INP9        ; Input bit set means input off, leave as zero in rawinps byte
        bsf     rawinps,0
        btfss   INP10
        bsf     rawinps,1
        btfss   INP11
        bsf     rawinps,2
        btfss   INP12
        bsf     rawinps,3
        btfss   INP13
        bsf     rawinps,4
        btfss   INP14
        bsf     rawinps,5

        ifndef  CE_BIT          ; Modules that have voltage doubler (CANACC4,CANCDU) do not have inputs 15 and 16
        btfss   INP15
        bsf     rawinps,6
        btfss   INP16
        bsf     rawinps,7
        endif

        movf    rawinps,w
        return

; **** Send any required events - w is map of inputs that need events sending, rawinps contains the status of the inputs ****

sndinpevs
        movwf   dbinputs            ; Save map of inputs to send
        movlw   ipidxfirs+1
        movwf   dbcount             ; Use same counter to track which event to send
inploop bcf     STATUS,C
        rrcf    dbinputs            ; Send an event for this input?
        bc      thisinp
        rrcf    rawinps             ; keep raw inputs (on/off status) aligned with map of inputs to send
        bra     nxtinp

thisinp rrcf    rawinps
        bnc     offevt              ; Is it on or off event?
        movlw   OPC_ACON
        bra     sendevt
offevt  movlw   OPC_ACOF
sendevt movwf   Tx1d0               ; set opcode to send
        clrf    Tx1d3
        movff   dbcount,Tx1d4       ; set event number to send
        movlw   5
        movwf   Dlc                 ; Set length for event packet
        call    sendTX              ; Send event

nxtinp  incf    dbcount
        movlw   16
        cpfsgt  dbcount
        bra     inploop

        movff   inputs,dbinputs     ; ready to debounce next change
        return


#ifndef		FLIM_ONLY

#ifdef     CANACE8MIO

;*****************************************************************************
;** SLiM NN assignement on CANACE8MIO

mioslimset
        btfss   PORTA,MIO_LEARN ; MIO learn mode?
        bra     mionewnn        ; Yes, go and get new id

; Get NN from EEPROM

   		movlw	LOW NodeID
		movwf	EEADR
		call	eeread
		movwf	NN_temph			;get stored NN
		incf	EEADR
		call	eeread
		movwf	NN_templ
        sublw   0                   ; Check for zero NN
        bnz     miosdun
        movf    NN_temph,w
        sublw   0
        bnz     miosdun
        clrf    NN_temph            ; Zero NN means not set yet,
        movlw   MIO_DEFAULT_NN      ; so set default value
        movwf   NN_templ
		movlw	Modstat             ;get setup status
		movwf	EEADR
		call	eeread
		bcf		Mode,0
        btfsc   eedata,1            ; ON only mode set?
		bsf		Mode,0              ;flag ON only
        bra     miosdun

; Set new NN from switches

mionewnn
        call    getopm          ; Get node number into temp
        incf    temp,w          ; node numbers count from 1
		clrf	NN_temph
		movwf	NN_templ

		movlw	Modstat         ; Ready to write flag for ON only to EEPROM
		movwf	EEADR

		bcf		Mode,0          ; ON/OFF mode
        movlw   0               ;flag for modstat in EEPROM for ON/OFF
		btfss	PORTA,MIO_ONONLY ;is it ON only?
        bra     onoff
		bsf		Mode,0			;flag ON only mode
        movlw   2               ;flag for modstat in EEPROM for ON only

onoff   call	eewrite         ; Set modstat flag and SLiM mode

call    putNNt                  ; save NN in EEPROM

miosdun bcf		Mode,1              ;not FLiM
        return


#else      ; SLiM for modules other than CANACE8MIO

;*************************************************************************
;**** CANMIO SLiM event learn/unlearn - uses values on rotary DIP switches ****

mioSLiM                         
        call    getopm          ; Get output number from switches into EVtemp (uses Temp,Temp1)
        call    getop3          ; Convert output number in Temp to output bit in EVtemp
        movlw   MIO_SLIM_MASK
        andwf   PORTA,w         ; Get learn switch bits
        movwf   switches

        sublw   MIOLRN          ; SLiM learn event
        bz      mlearn          ; yes
        movlw   MIOLRNINV       ; SLiM learn with inverted polarity
        cpfseq  switches        ; yes
        bra     chkunl

mlearn  call    enmatch         ; Uses Temp,returns zero if found
        sublw   0
        bnz     newevm
    	call	rdfbev			;read events data

   ; Event found, so update it with this teaching

upev    movf	EVtemp,W       ; Get selected output
        iorwf	POSTINC0,F     ; Combine with outputs already taught
;        movwf	POSTINC0        ;write back to EV
;        movwf	EVtemp          ;save for testing
        btfsc   switches,MIO_POL ; check polarity
        iorwf   INDF0,F          ; If switch set for polarity, or in polarity bit
       	call	wrfbev          ;write back to flash
        bra     miosdun          ; SLiM learn finished

newevm  call    createv         ; create new ev entry
        sublw   0               ; succcess?
        bz		upev            ; yes, do the event variables      

chkunl  movf    switches,W      ; Forget (unlearn) this event
        sublw   MIOFGET
        bnz     miosdun          ; nothing else to do
        call    unlearn

miosdun goto    l_out2

#endif  ; CANACE8MIO

; Version of Getop for CANMIO
; Gets which output selected by DIP switches and put into Temp
; Also used to get node id for CANACE8MIO
; Uses Temp and Temp1. returns result in temp

getopm  movlw	B'00110011'		;get DIP switch setting for output
		andwf	PORTB,W
		movwf	Temp
		movwf	Temp1
		rrncf	Temp1,F
		rrncf	Temp1,w
		andlw	B'00001100'
		iorwf	Temp,F          ; LS 4 bits are now switches
        comf    Temp            ; Invert so switch on is now 1 and switch off is now 0
        movlw   B'00001111'
        andwf   Temp,F
        decf    Temp            ; So that 1 to 8 on switches gives 0-7
        return          ; Continue routine with common code in evhndlr


#endif ; not FLiM only
       



