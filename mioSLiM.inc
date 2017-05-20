
; I/O port bit definitions for SLiM operation of CANMIO range

; (c) Pete Brownlow  2015

; NOTE: To display this file with everything correctly lined up set your editor or IDE tab stop to 4

; These definitions are common across CANMIO, CANBIP and CANCDU hardware

MIO_DEFAULT_NN  equ .129    ; Default NN for a CANMIO producer in SLiM where node number has not been set

; PORT A bits

MIO_LEARN       equ 5       ; RA5 will be low during any MIO learn operation
MIO_POL         equ 1       ; RA1 decides polarity during learn
MIO_DEL         equ 3       ; RA3 is high during forget and erase operations
MIO_PAIR        equ 0       ; RA0 is low when teaching pairs of outputs

MIO_ONONLY      equ 1       ; RA1 defines on-only mode for CANACE8MIO in SLiM


; Rotary DIP switch S1 settings for learn operations

#ifdef CANCDU                    ; Less bits to play with on CANCDU
MIO_SLIM_MASK   equ B'00000011'  ; Mask for bits in port A for rotary SLiM coniguration switch
MIO_OP_MASK     equ B'00110011'  ; Mask for output selction on second rotary switch
#else
MIO_SLIM_MASK   equ B'00101011'  ; Mask for bits in port A for rotary SLiM coniguration switch
MIO_OP_MASK     equ B'00010011'  ; Mask for output selction on second rotary switch
#endif


; Actual values read from Port A after AND with mask

; The value set on the rotary DIP switches is mapped into spare inputs on the PIC
; This table works out the actual values to compare to in the code

                                ; Switch        Inverted    Mapped
                                ; Code  Binary  Active low  to inputs   Usage

MIOLRN          equ B'00001010'  ; A -   1010 -  0101        01010       Learn
MIOLRNINV       equ B'00001000'  ; B -   1011 -  0100        01000       Learn inverted polarity
MIOLRNCNV       equ B'00000011'  ; C -   1100 -  0011        00011       Learn pair converse (ie: inverted polarity)
MIOLRNDIR       equ B'00000001'  ; D -   1101 -  0010        00001       Learn pair direct
MIOERASE        equ B'00000010'  ; E -   1110 -  0001        00010       Erase all on power on
MIOFGET         equ B'00000000'  ; F -   1111 -  0000        00000       Forget one event



