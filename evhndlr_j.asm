; filename evhndlr_h.asm
; modified 05/12/11 - fix SLiM mode learning
; modified 06/12/11 - add learnin label
; 					- add movwf TABLAT to nxtfb routine for 32nd byte
; Rev d 09/03/12	- add checks on EN index and EV index in evsend, correct error code
; Rev d 22/04/12	- remove off/on of LEDs in wrflsh routine
; Rev f             - added LOW before EEPROM addresses. Prevents error messages
; Rev g  23/08/12	 - Extra code in 'isthere' for event delete in CANSERVO8C
; Rev h 08/08/15 pnb - Refactored newev as a subroutine createv that can be called from elsewhere
; Rev j 18/2/17  pnb - Make compatible with 25k80 by clearing EEADRH before EEPROM ops (only uses first 256 bytes of EEPROM except for high byte for bootloader)        

;include file for handling flash event data

;************************************************************************
		
;		send number of events

evnsend
        clrf    EEADRH
		movlw	LOW ENindex+1
		movwf	EEADR
		call	eeread
		movwf	Tx1d3
		movlw	0x74
		movwf	Tx1d0
		movlw	4
		movwf	Dlc
		call	sendTX
		return

;**************************************************************************
;
;	learn an event
;
;	The event must be in ev0, ev1, ev2 & ev3
;	The EV index in EVidx and the value in EVdata
;
; define FLIM_ONLY for CANSERVO

learnin
#ifdef	FLIM_ONLY
	bra	lrnin1
#else
	btfsc	Mode,1
	bra		lrnin1
	btfss	PORTA,UNLEARN	;don't do if unlearn
	retlw	0
	bra		learn
#endif
	
lrnin1
	btfsc	Datmode, 5		;don't do if unlearn
	retlw	0

learn		; data is in ev(n), EVdata and EVidx
	call	enmatch	
	sublw	0
	bnz		newev
	call	rdfbev			;read events data
		
	; sort out EVs here
lrnevs	
	btfsc	initFlags,0
	bra		lrnevs1		;j if initialising event data
#ifndef		FLIM_ONLY
	btfss	Mode,1		;FLiM mode?
	bra		lrns
#endif

lrnevs1
	movf	EVidx,w		;FLiM mode or init, just write new ev to event data
	movff	EVdata, PLUSW0
	call	wrfbev		; write back to flash
	btfsc	initFlags,0
	retlw	0			;dont send WRACK on initialisation
	movlw	0x59
	call	nnrel		;send WRACK
	retlw	0

#ifndef	FLIM_ONLY	
lrns				;learn event values in SLiM mode
	call	getop	;returns switch data in EVtemp
	movff	EVtemp, EVtemp1	;make a copy
	movf	EVtemp1,W
	iorwf	INDF0,W
	movwf	POSTINC0	;write back to EV
	movwf	EVtemp		;save for testing
	btfsc	PORTB, POL
	bra		lrns2
	movf	EVtemp1,w	;recover output bit
	iorwf	INDF0		;or in POL bit
lrns1
	movff	INDF0,EVtemp2		;save for testing
	call	wrfbev		;write back to flash
	retlw	0
	
lrns2
	comf	EVtemp1,w
	andwf	INDF0		;remove POL bit
	bra		lrns1
#endif
	
newev
    call    createv     ; create new ev entry
    sublw   0           ; succcess?
    bz		lrnevs      ; yes, do the event variables
    retlw   1           ; no - return from learnin

createv ; Create new event in hash table
        ; Returns 0 for success, 1 for table full

	; check remaining space
	movlw	LOW ENindex
	movwf	EEADR
	call	eeread
	sublw	0
	bnz		lrn1
	retlw	1		; no space left
	
lrn1	
	movlw	LOW FreeCh	; get free chain pointer
	movwf	EEADR
	call	eeread
	movwf	evaddrh
	movlw	LOW FreeCh + 1
	movwf	EEADR
	call	eeread
	movwf	evaddrl
	
	; now check and update hash table pointer
	tstfsz	htaddrh
	bra		lrnhtok
	bra		newev2		;j if no hash table for this event

lrnhtok				; hash table pointer is valid so read data
	call	rdfbht		; read from hash table address
	movf	htaddrl, w
	call	setFsr0
	movlw	6
	addwf	FSR0L
	movff	evaddrh, POSTINC0
	movff	evaddrl, POSTINC0
	call	wrfbht		; write back using hash table address
	
newev2		; read free chain data block
	call	rdfbev		; read free chain entry	
	movf	evaddrl, w
	call	setFsr0
	movlw	4
	addwf	FSR0L		;point at nextnh
	
	; now update FreeCh with next event pointer from event data
	movlw	LOW FreeCh
	movwf	EEADR
	movf	POSTINC0,W
	call	eewrite
	
	movlw	LOW FreeCh+1
	movwf	EEADR
	movf	POSTINC0,W
	call	eewrite	
	
	; write new event address to hash table
	movff	htidx, EEADR
	movf	evaddrh,w
	call	eewrite
	incf	EEADR
	movf	evaddrl,w
	call	eewrite

	movf	evaddrl, W
	call	setFsr0
	movff	ev0,POSTINC0
	movff	ev1,POSTINC0
	movff	ev2,POSTINC0
	movff	ev3,POSTINC0
	movff	htaddrh, POSTINC0		; copy previous head of chain address
	movff	htaddrl, POSTINC0
	clrf	POSTINC0				; clear previous ptr
	clrf	POSTINC0
	
	movlw	.8
	movwf	CountFb0
	movlw	0
	
newev3
	clrf	PLUSW0			; clear event data, leave FSR0 alone
	incf	WREG
	decfsz	CountFb0
	bra		newev3
	
	movf	hnum,w				; hash number of event
	addlw	LOW hashnum			; update count of events in this hash
	movwf	EEADR
	call	eeread
	incf	WREG
	call	eewrite
	
	movlw	LOW ENindex + 1		;update number of events
	movwf	EEADR
	call	eeread
	addlw	1
	call	eewrite
	decf	EEADR
	call	eeread
	decf	WREG
	call	eewrite				;update remaining space
    retlw   0                   ; 0 indicates event created successfully


;**************************************************************************
;
;	unlearn an event
;	The event must be in ev0, ev1, ev2 & ev3

unlearn			; on entry the target event number must be in ev0-3
	call	enmatch
	sublw	0
	bz		unl1			; j if event found
	return
	
unl1
	movlw	LOW FreeCh		; get free chain address
	movwf	EEADR
	call	eeread
	movwf	freadrh
	movlw	LOW FreeCh+1
	movwf	EEADR
	call	eeread
	movwf	freadrl
	
	call	rdfbev				; read entry
	movf	evaddrl,W
	call	setFsr0				;point FSR0 at relevant data
	movlw	4
	addwf	FSR0L				;adjust to point at nextnh
	movff	INDF0, nextadrh		;save chain pointers
	movff	freadrh,POSTINC0	; set next ptr to current free chain
	movff	INDF0, nextadrl		; ditto with ls addr
	movff	freadrl, POSTINC0
	movff	INDF0, prevadrh		;save prevnh
	clrf	POSTINC0			; clear previous entry ponter
	movff	INDF0, prevadrl		;save prevnl
	clrf	POSTINC0

	movlw	LOW FreeCh		; update free chain address to current entry
	movwf	EEADR
	movf	evaddrh,w
	call	eewrite
	movlw	LOW FreeCh+1
	movwf	EEADR
	movf	evaddrl,w
	call	eewrite
	
	call	wrfbev			; write freed event data back
	
	tstfsz	prevadrh		; check if previous link id valid
	bra		unl2			; j if it is
	bra		unl3

unl2						; read and update previous event entry
	movff	prevadrh, evaddrh
	movff	prevadrl, evaddrl
	call	rdfbev
	movf	evaddrl,W
	call	setFsr0				;point FSR0 at relevant data
	movlw	4
	addwf	FSR0L
	movff	nextadrh, POSTINC0
	movff	nextadrl, POSTINC0
	call	wrfbev			; write back with updated next pointer
	
unl3						;must write next ptr to hash table
	movff	htidx, EEADR
	movf	nextadrh,w
	call	eewrite
	incf	EEADR
	movf	nextadrl,w
	call	eewrite

	tstfsz	nextadrh		; check if next link is valid
	bra		unl4			; j if it is
	bra		unl5			; no more to do
	
unl4
	movff	nextadrh, evaddrh
	movff	nextadrl, evaddrl
	call	rdfbev
	movf	evaddrl,W
	call	setFsr0				;point FSR0 at relevant data
	movlw	6
	addwf	FSR0L				;adjust to point at prevnh
	movff	prevadrh, POSTINC0
	movff	prevadrl, POSTINC0
	call	wrfbev

unl5
	movf	hnum, w				; hash number of event
	addlw	LOW hashnum			; update number of events for this hash
	movwf	EEADR
	call	eeread
	decf	WREG
	call	eewrite

	movlw	LOW ENindex + 1			;update no of events and free space 
	movwf	EEADR
	call	eeread
	decf	WREG
	call	eewrite					;no of events
	decf	EEADR
	call	eeread
	addlw	1
	call	eewrite					;free space
	return
	
;**************************************************************************
;
; copy all EEPROM events to Flash if FLASH not initialised.
; initialises Free chain and moves all existing events to Flash
; 

copyEVs
	clrf	initFlags
	movlw	LOW ENindex
	movwf	EEADR
	call	eeread			;get free space
	movwf	Temp
	incf	EEADR
	call	eeread			;get num of events
	addwf	Temp			;total should equal EVT_NUM
	movlw	EVT_NUM
	cpfseq	Temp
	bra     noFreeCh
	return					;Free chain set up so do nothing
	
noFreeCh
	call	en_ram			;copy events from EEPROM to RAM
	movlw	LOW ENindex+1	;get number of events
	movwf	EEADR
	call	eeread			;number of events
	movwf	ENcount			;save count
	
	call	initevdata		;create free chain
	
	tstfsz	ENcount			;check if any events to copy
	bra		docopy			;j if there are...
	return					;...else no more to do
	
docopy
	bsf		initFlags,0		;for learn routine
	lfsr	FSR1, EN1
	lfsr	FSR2, EV1
	
cynxten
	movff	POSTINC1, ev0
	movff	POSTINC1, ev1
	movff	POSTINC1, ev2
	movff	POSTINC1, ev3
	movlw	0				; index of first EV
	movwf	EVidx
	movff	POSTINC2, EVdata
	call 	learn
	movlw	1				; index of second EV
	movwf	EVidx
	movff	POSTINC2, EVdata
	call	learn
	decfsz	ENcount
	bra		cynxten
	
	clrf	initFlags
	return
	

;**************************************************************************
;
; clear all events and associated data structures
; Each event is stored as a 16 byte entity
; Bytes 0-3 are the event number
; Bytes 4-5 are the pointer to the next entry, 0 if none
; Bytes 6-7 are the pointer to the previous entry, 0 if none
; Bytes 8-15 contain the events data, usage depends on the actual module
;
; creates the free chain and initialises next and previous pointers
; in each entry
; clears the event data for each event in the free chain

initevdata		; clear all event info				
	movlw	LOW ENindex+1		;clear number of events
	movwf	EEADR
	movlw	0
	call	eewrite			; no events set up
	movlw	EVT_NUM
	decf	EEADR
	call	eewrite			; set up no of free events

	movlw	HASH_SZ * 2
	movwf	Count	
	movlw	LOW hashtab
	movwf	EEADR
	
nextht						; clear hashtable
	movlw	0
	call	eewrite
	incf	EEADR
	decfsz	Count
	bra		nextht
	
	movlw	HASH_SZ
	movwf	Count
	movlw	LOW hashnum
	movwf	EEADR
	
nexthn						; clear hash table count
	movlw	0
	call	eewrite
	incf	EEADR
	decfsz	Count
	bra		nexthn
	
	call	clrev		; erase all event data
	movlw	LOW FreeCh	; set up free chain pointer in ROM
	movwf	EEADR
	movlw	HIGH evdata
	movwf	evaddrh
	call	eewrite
	movlw	LOW FreeCh + 1
	movwf	EEADR
	movlw	0
	movwf	evaddrl
	call	eewrite
	call	clrfb
	clrf	prevadrh
	clrf	prevadrl
	
	movlw	EVT_NUM
	movwf	Count		; loop for all events

nxtblk
	movff	evaddrl, Temp
	movff	evaddrh, Temp1
nxtevent
	movf	Temp,W
	call	setFsr0
	movlw	.16
	addwf	Temp
	movlw	0
	addwfc	Temp1
	clrf	POSTINC0
	clrf	POSTINC0
	clrf	POSTINC0
	clrf	POSTINC0
	decf	Count
	bz		lastev		; j if final event
	movff	Temp1,POSTINC0
	movff	Temp, POSTINC0
	movff	prevadrh, POSTINC0
	movff	prevadrl, POSTINC0
	movff	evaddrh, prevadrh
	movff	evaddrl, prevadrl
	movf	Count,w
	andlw	0x03
	bz		nxtevent1
	movlw	.16		
	addwf	evaddrl		; move on to next 16 byte block
	movlw	0
	addwfc	evaddrh	
	bra		nxtevent
nxtevent1
	call	wrfbev
	movlw	.16		
	addwf	evaddrl		; move on to next 64 byte block
	movlw	0
	addwfc	evaddrh	
	bra		nxtblk
	
lastev	
	clrf	Temp
	movff	Temp,POSTINC0
	movff	Temp,POSTINC0
	movff	prevadrh,POSTINC0
	movff	prevadrl, POSTINC0
	call	wrfbev
	return

	
;**************************************************************************
;
; routines for reading the event data

rdfbht		; on entry htaddrh and htaddrl must point to valid entry
	movlw	0xc0
	andwf	htaddrl,w		; align to 64 byte boundary
	movwf	TBLPTRL
	movf	htaddrh,w
	movwf	TBLPTRH
	movlw	.64
	movwf	CountFb0
	bra		rdfb
	
rdfbev		; On entry evaddrh and evaddrl must point to the correct entry
	movlw	0xc0
	andwf	evaddrl,w		; align to 64 byte boundary
	movwf	TBLPTRL
	movf	evaddrh,w
	movwf	TBLPTRH
	clrf	TBLPTRU
	movlw	.64
	movwf	CountFb0
rdfb
	clrf	TBLPTRU
	movff	FSR0H, Saved_Fsr0H	;must preserve FSR0
	movff	FSR0L, Saved_Fsr0L
	lfsr	FSR0,evt00
nxtfbrd
	tblrd*+
	movf	TABLAT, w
	movwf	POSTINC0
	decfsz	CountFb0
	bra		nxtfbrd
	movff	Saved_Fsr0L, FSR0L	;recover FSR0
	movff	Saved_Fsr0H, FSR0H
	return
	
rdfbev16		; read 16 bytes of event data, on entry evaddrh and evaddr must be valid
	clrf	TBLPTRU
	movff	evaddrh, TBLPTRH
	movff	evaddrl, TBLPTRL
	movlw	.16
	movwf	CountFb0
	bra		rdfb
	
rdfbev8		; read first 8 bytes of event data, on entry evaddrh and evaddr must be valid
	clrf	TBLPTRU
	movff	evaddrh, TBLPTRH
	movff	evaddrl, TBLPTRL
	movlw	8
	movwf	CountFb0
	bra		rdfb

;**************************************************************************
;
;	routine for finding an event - returns 0 on success, 1 on failure
;	If successful, FSR0 points at the event data

enmatch		;on exit if success w = 0 and evaddrh/evaddrl point at led data
	movlw	0x07
	andwf	ev3,w		;ls 3 bits as hash
	movwf	Temp
	movwf	hnum
	rlncf	Temp		; times 2 as hash tab is 2 bytes per entry	
	movlw	LOW hashtab
	addwf	Temp, w
	movwf	htidx		; save EEPROM offset of hash tab entry
	movwf	EEADR
	call	eeread
	movwf	evaddrh
	movwf	htaddrh		; save hash table point ms
	incf	EEADR
	call	eeread
	movwf	evaddrl
	movwf	htaddrl		; save hash table pointer ls
nextev
	tstfsz	evaddrh		;is it zero?, high address cannot be zero if valid
	bra		addrok
	retlw	1			; not found
	
addrok
	movf	evaddrl,w
	movwf	TBLPTRL
	movf	evaddrh,w
	movwf	TBLPTRH
	clrf	TBLPTRU
	
	clrf	Match
	tblrd*+
	movf	TABLAT,W
	cpfseq	ev0
	incf	Match
	tblrd*+
	movf	TABLAT,W
	cpfseq	ev1
	incf	Match
	tblrd*+
	movf	TABLAT,W
	cpfseq	ev2
	incf	Match
	tblrd*+
	movf	TABLAT,W
	cpfseq	ev3
	incf	Match
	tstfsz	Match
	bra		no_match
	movf	evaddrl,w
	call	setFsr0
	movlw	8
	addwf	FSR0L		;leave FSR0 pointing at EVs
	retlw	0
	
no_match		;get link address to next event
	tblrd*+
	movf	TABLAT,w
	movwf	evaddrh
	tblrd*+
	movf	TABLAT,w
	movwf	evaddrl
	bra		nextev
	


;**************************************************************************
;
; routines for clearing flash ram
	
clrev		; erase all of the event data area in flash
	movlw	EVT_NUM/4
	movwf	Count		; 4 events per 64 bytes
	movlw	LOW evdata
	movwf	TBLPTRL
	movlw	high evdata
	movwf	TBLPTRH
	clrf	TBLPTRU
nxtclr
	call	clrflsh
	decfsz	Count
	bra		nxtblock
	return
nxtblock
	movlw	.64
	addwf	TBLPTRL,F
	movlw	0
	addwfc	TBLPTRH
	bra		nxtclr
	
clrflsh		; clears 64 bytes of flash ram, TBLPTR must point to target ram
	bsf		EECON1,EEPGD		;set up for erase
	bcf		EECON1,CFGS
	bsf		EECON1,WREN
	bsf		EECON1,FREE
	movff	INTCON,TempINTCON
	clrf	INTCON	;disable interrupts
	movlw	0x55
	movwf	EECON2
	movlw	0xAA
	movwf	EECON2
	bsf		EECON1,WR			;erase
	nop
	nop
	nop
	movff	TempINTCON,INTCON	;reenable interrupts

	return
	
clrfb		; clears the 64 bytes data ram for ev data
	lfsr	FSR0, evt00
	movlw	.64
	movwf	CountFb0
nxtone
	clrf	POSTINC0
	decfsz	CountFb0
	bra		nxtone
	return

;**************************************************************************
;
;	routines for writing flash
;
; erases flash ram and the writes data back
; writes the 64 bytes of data ram back to flash ram

wrfbht
	; htaddrh and htaddrl must contain the flash address on entry
	movlw	0xc0
	andwf	htaddrl,w		; align to 64 byte boundary
	movwf	TBLPTRL
	movf	htaddrh, W
	movwf	TBLPTRH
	bra		wrfb
	
	; erases flash ram and the writes data back
	; writes the 64 bytes of data ram back to flash ram
	
	; evaddrh and evaddrl must contain the flash address on entry
wrfbev
	movlw	0xc0
	andwf	evaddrl,w		; align to 64 byte boundary
	movwf	TBLPTRL
	movf	evaddrh, W
	movwf	TBLPTRH
	
wrfb
	clrf	TBLPTRU
	call	clrflsh
	lfsr	FSR0, evt00
	movlw	2
	movwf	CountFb1
nxt32
	movlw	.31			;first 31 bytes
	movwf	CountFb0
nxtfb
	movf	POSTINC0, W
	movwf	TABLAT
	tblwt*+
	decfsz	CountFb0
	bra 	nxtfb
	movf	POSTINC0,W
	movwf	TABLAT
	tblwt*				; must leave TBLPTR pointing into 32 byte block
	call 	wrflsh
	incf	TBLPTRL		;now move into next 32 byte block
	movlw	0
	addwfc	TBLPTRH
	decfsz	CountFb1
	bra		nxt32
	return
	
wrflsh		; write upto 32 bytes of flash	
	bsf		EECON1, EEPGD
	bcf		EECON1,CFGS
	bcf		EECON1,FREE			;no erase
	bsf		EECON1, WREN
;	bsf		PORTC,2				;LEDs off
	movff	INTCON,TempINTCON
	clrf	INTCON
	movlw	0x55
	movwf	EECON2
	movlw	0xAA
	movwf	EECON2
	bsf		EECON1,WR
	nop
	movff	TempINTCON,INTCON
;	bcf		PORTC,2				;LEDs on
	return
	
;**************************************************************************
;
; setFsr0 
; Om entry w contains the Flash address of the event
; FSR0 is set to point to the correct block of event data in RAM
; data is not copied to ram by this routine

setFsr0
	swapf	WREG
	andlw	0x03
	tstfsz	WREG
	bra 	nxtev1
	lfsr	FSR0, evt00
	return
nxtev1
	decf	WREG
	tstfsz	WREG
	bra		nxtev2
	lfsr	FSR0, evt10
	return
nxtev2
	decf	WREG
	tstfsz	WREG
	bra		nxtev3
	lfsr	FSR0, evt20
	return
nxtev3
	lfsr	FSR0, evt30
	return
		
		

;**************************************************************************
;
; read event variable, ev0-3 must be set
	
readev
	call	enmatch
	sublw	0
	bz		readev1			; j if event found
	clrf	EVdata
	clrf	EVidx
	bra		endrdev
	
readev1
	call	rdfbev16		; get 16 bytes of event data
	lfsr	FSR0, ev00		; point at EVs
	movf	EVidx,w
	movf	PLUSW0,w		; get the byte
	movwf	EVdata
	incf	EVidx			; put back to 1 based
	
endrdev
	movlw	0xD3
	movwf	Tx1d0
	movff	evt00,Tx1d1
	movff	evt01,Tx1d2
	movff	evt02,Tx1d3
	movff	evt03,Tx1d4
	movff	EVidx,Tx1d5
	movff	EVdata,Tx1d6
	movlw	7
	movwf	Dlc
	call	sendTXa
	return
	
;*************************************************************************

;		read back all events in sequence

enread	clrf	Temp
		movlw	LOW ENindex+1
		movwf	EEADR
		call	eeread
		sublw	0
		bz		noens		;no events set
		
		clrf	Tx1d7		; first event
		movlw	HASH_SZ			; hash table size
		movwf	ENcount	
		movlw	LOW hashtab
		movwf	EEADR
nxtht
		call	eeread
		movwf	evaddrh
		incf	EEADR
		call	eeread
		movwf	evaddrl
nxten
		tstfsz	evaddrh		; check for valid entry
		bra		evaddrok
nxthtab
		decf	ENcount
		bz		lasten
		incf	EEADR
		bra		nxtht
		
evaddrok					; ht address is valid
		call	rdfbev8		; read 8 bytes from event info
		incf	Tx1d7
		movff	evt00, Tx1d3
		movff	evt01, Tx1d4
		movff	evt02, Tx1d5
		movff	evt03, Tx1d6
		
		movlw	0xF2
		movwf	Tx1d0
		movlw	8
		movwf	Dlc
		call	sendTX
		call	dely
		call	dely
		
		movff	next0h, evaddrh
		movff	next0l,	evaddrl
		bra		nxten

noens	movlw	7				;no events set
		call	errsub
		return
	
lasten	return	

;*************************************************************************
;
;	findEN - finds the event by its index

findEN					; ENidx must be valid and in range
		clrf	hnum
		clrf	ENcount
		clrf	ENcount1
findloop
		movf	hnum,w
		addlw	LOW hashnum
		movwf	EEADR
		call	eeread
		addlw	0
		bz		nxtfnd
		addwf	ENcount1
		movf	ENcount1,w
		cpfslt	ENidx		;skip if ENidx < ENcount1
		bra		nxtfnd
		bra		htfound
nxtfnd
		movff	ENcount1, ENcount
		incf	hnum
		bra		findloop
htfound
		rlncf	hnum,w
		addlw	LOW hashtab
		movwf	EEADR
		call	eeread
		movwf	evaddrh
		incf	EEADR
		call	eeread
		movwf	evaddrl
nxtEN
		movf	ENidx,w
		cpfslt	ENcount
		return
		
nxtEN1
		incf	ENcount
		call	rdfbev8
		movff	next0h, evaddrh
		movff	next0l, evaddrl
		bra		nxtEN
		
;*************************************************************************

;	send individual event by index, ENidx must contain index

enrdi	movlw	LOW ENindex+1	; no of events set
		movwf	EEADR
		call	eeread
		sublw	0
		bz		noens1		;no events set
		
		movf	ENidx,w		; index starts at 1
		bz		noens1
		
		decf	ENidx		; make zero based for lookup
		movlw	LOW ENindex +1
		movwf	EEADR
		call	eeread		; read no of events
		cpfslt	ENidx		; required index is in range
		bra		noens1
		
		call	findEN
		call	rdfbev8		; get event data
		
		movff	evt00, Tx1d3
		movff	evt01, Tx1d4
		movff	evt02, Tx1d5
		movff	evt03, Tx1d6
		incf	ENidx
		movff	ENidx, Tx1d7
		movlw	0xF2
		movwf	Tx1d0
		movlw	8
		movwf	Dlc
		call	sendTX
		return
		
noens1	movlw	7				;inavlid event index
		call	errsub
		return
		
;***********************************************************

;		send EVs by reference to EN index, ENidx must be set

evsend
		movlw	LOW ENindex+1	; no of events set
		movwf	EEADR
		call	eeread
		sublw	0
		bz		noens1		;no events set
		
		movf	ENidx,w		; index starts at 1
		bz		noens1
		
		decf	ENidx		; make zero based for lookup
		movlw	LOW ENindex+1
		movwf	EEADR
		call	eeread		; read no of events
		cpfslt	ENidx		; required index is in range
		bra		noens1
		
		movf	EVidx,w		; index starts at 1
		bz		notEV		; zero is invalid
		decf	EVidx
		movlw	EVperEVT
		cpfslt	EVidx		; skip if in range
		bra		notEV
		
		call	findEN
		
		call	rdfbev16	; read event data
		lfsr	FSR0,ev00
		movf	EVidx,w
		movff	PLUSW0, Tx1d5
		incf	EVidx		; make 1 based again...
		incf	ENidx		; ... ditto
		movlw	0xB5
		movwf	Tx1d0
		movff	ENidx,Tx1d3
		movff	EVidx,Tx1d4
		movlw	6
		movwf	Dlc
		call	sendTX
		return

notEV	movlw	6		;invalid EN#
		call	errsub
		return

;***********************************************************

;		send free event space


rdFreeSp
		movlw	LOW ENindex		;read free space
		movwf	EEADR
		call	eeread
		movwf	Tx1d3
		movlw	0x70
		movwf	Tx1d0
		movlw	4
		movwf	Dlc
		call	sendTX
		return
		
;************************************************************
;
; End of evhndlr_h.asm
;
;************************************************************
