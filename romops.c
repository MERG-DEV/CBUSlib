/*
 romops.c - EEPROM and FLASH ROM routines - part of CBUS libraries for PIC 18F


 Original CANACC8 assembler version (c) Mike Bolton
 Modifications to EEPROM routines and conversion to C18 (c) Andrew Crosland
 FLASH routines by (c) Chuck Hoelzen
 Modifications, refinements & combine EEPROM and FLASH into one module (C) Pete Brownlow 2014-2017   software@upsys.co.uk

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

 Ported to XC8 by Ian Hogg 23/5/2017

*/
/**
 * Flash routines hide the complexity of erasing and writing in pages.
 * A buffer is kept of the current page being changed and then writing it back
 * in a single operation. This reduces the number of writes to each page of flash
 * and extending its life.
 */
#include "devincs.h"
#include "romops.h"
#include "EEPROM.h"
#include "hwsettings.h"

//#pragma romdata BOOTFLAG
//rom BYTE bootflag = 0;

#ifndef __XC8__
#pragma udata MAIN_VARS
#endif

FlashFlags  flashFlags;
BYTE        flashbuf[_FLASH_WRITE_SIZE];    // Assumes that Erase and Write are the same size
BYTE        flashidx;
WORD        flashblock;                     //address of current 64 byte flash block

#ifndef __XC8__
#pragma code APP
#endif

// Internal function definitions

void writeFlashShort(void);
void writeFlashWithErase(void);
BYTE readFlashBlock(WORD flashAddr);

/**
 *  Initialise variables for Flash program tracking.
 */
void initRomOps() {
    flashFlags.asByte = 0;
    flashblock = 0xFFFF;
}


/**
 * Flash block write. flash 64 byte buffer.
 * Fast write, this requires no 0 to 1 bit changes, only 1 to 0 bits are allowed. 
 * Or use write_flash_long with erase before write 
 */
 void  writeFlashShort(void) {
    
#ifdef __XC8__
    TBLPTR = flashblock & ~(64 - 1); //force row boundary
    di();     // disable all interrupts ERRATA says this is needed before TBLWT
    for (unsigned char i=0; i<64; i++) {
        TABLAT = flashbuf[i];
        asm("TBLWT*+");
    }
    // Note from data sheet: 
    //   Before setting the WR bit, the Table
    //   Pointer address needs to be within the
    //   intended address range of the 64 bytes in
    //   the holding register.
    // So we put it back into the block here
    TBLPTR = flashblock & ~(64 - 1);
    EECON1bits.EEPGD = 1;   // 1=Program memory, 0=EEPROM
    EECON1bits.CFGS = 0;    // 0=ProgramMemory/EEPROM, 1=ConfigBits
    EECON1bits.FREE = 0;    // No erase
    EECON1bits.WREN = 1;    // enable write to memory
    
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = TRUE;
    ei();     // enable all interrupts
    EECON1bits.WREN = FALSE;
#else
    WORD ptr;
    BYTE fwCounter;
    di();     // disable all interrupts
    
    ptr= (WORD)flashbuf;
    TBLPTR=flashblock;
    FSR0=ptr;
    

#ifdef CPUF18K
    flashidx=64;            // K series processors can write 64 bytes in one operation
#else
    for (fwCounter = 1; fwCounter <=2; fwCounter++ )    // 18F processors need two iterations of 32 bytes each
    {
         flashidx=32;
#endif

_asm
L_w:
        MOVF POSTINC0,W,0
        MOVWF TABLAT,0
        TBLWTPOSTINC
        DECF flashidx,F,1
        BNZ L_w
        TBLRDPOSTDEC            //PUT IT BACK IN THE BLOCK
_endasm

        EECON1bits.EEPGD = 1;   // 1=Program memory, 0=EEPROM
        EECON1bits.CFGS = 0;    // 0=ProgramMemory/EEPROM, 1=ConfigBits
        EECON1bits.FREE = 0;    // No erase
        EECON1bits.WREN = 1;    // enable write to memory
        EECON2 = 0x55;          // write 0x55
        EECON2 = 0xaa;          // write 0xaa
        EECON1bits.WR = 1;      // start writing
        EECON1bits.WREN = 0;    // disable write to memory
        
_asm
        TBLRDPOSTINC            // Table pointer ready for next 32
_endasm
        ei();     // enable all interrupts
#endif

#ifdef CPUF18F
    }
#endif
    
}


/**
 * Flash block write. flash 64 byte buffer with block erase.
 */
void  writeFlashWithErase(void) {
    // Erase block first
    TBLPTR=flashblock;
    EECON1bits.EEPGD = 1;   // 1=Program memory, 0=EEPROM
    EECON1bits.CFGS = 0;    // 0=Program memory/EEPROM, 1=ConfigBits
    EECON1bits.WREN = 1;    // enable write to memory
    EECON1bits.FREE = 1;    // enable row erase operation
    di();     // disable all interrupts
    EECON2 = 0x55;          // write 0x55
    EECON2 = 0xaa;          // write 0xaa
    EECON1bits.WR = 1;      // start erasing
    ei();     // enable all interrupts
    EECON1bits.WREN = 0;    // disable write to memory
    //Now write data to flash
    writeFlashShort();
}

/**
 * If the buffer has unwritten changes then write these out to Flash.
 */
 void flushFlashImage( void ) {
     if (flashFlags.modified) {
        if(flashFlags.zeroto1) {
            writeFlashWithErase();
        } else {
            writeFlashShort();
        }
     }
 }


/**
 * Flash block read. Read flash through a 64 byte buffer with write back management.
 *  valid:3    //must be 101 (5) to be valid
 *  loaded:1   // if buffer is loaded
 *  modified:1 //flag if buffer is modified
 *  zeroto1:1  //flag if long write with block erase
 */
 /**
 * Read a byte from flash. If the required buffer is currently loaded then use the value 
 * stored there since it may have been modified. Otherwise load the buffer from flash
  * before returning the value.
 * @param addr the address to be read from Flash
 * @return the byte read from Flash
 */
BYTE readFlashBlock(WORD flashAddr) {
    WORD ptr;

    if(flashFlags.valid !=5) {
        flashFlags.asByte=5;  //force reload
    }

    if(flashFlags.loaded && flashblock!=(flashAddr & 0XFFC0)) {
        //detected access from a different block so we need to write this one (if it has been changed)
        flushFlashImage();
        flashFlags.asByte=5;
    }

    if(!flashFlags.loaded) {
        // load the buffer
        flashblock = flashAddr & 0xFFC0;
#ifdef __XC8__
        EECON1=0X80;    // access to flash
        TBLPTR = flashblock;
        for (unsigned char i=0; i<64; i++) {
            asm("TBLRD*+");
            flashbuf[i] = TABLAT;
        }
#else
        //load the buffer
        ptr= (WORD)flashbuf;
        FSR0=ptr;
        flashblock = flashAddr & 0XFFC0;
        TBLPTR=flashblock;
        EECON1=0X80;
        flashidx=64;
_asm
//        MOVLB FLASHBUFPAGE
READ_BLOCK:
        TBLRDPOSTINC
        MOVFF TABLAT,POSTINC0
        DECF flashidx,F,1
        BNZ READ_BLOCK
_endasm
#endif
        flashFlags.loaded = TRUE;
    }
    return flashbuf[flashAddr & 0X3F];
}


/**
 * Write a byte to the FLASH image. You may need to flush current image to Flash if necessary.
 * @param addr the destination address of the byte to be written
 * @param data the data byte to be written
 */
void writeFlashImage(BYTE * addr, BYTE data) {
    unsigned char *offset;

    if(flashFlags.valid !=5) {
        flashFlags.valid=5;  //force reload
    }

    if (!flashFlags.loaded || flashblock!=((WORD)addr & 0XFFC0)) {
        readFlashBlock((WORD)addr);
    }
    offset = &flashbuf[(WORD)addr & 0x3F];

    if(data !=*offset) {
        flashFlags.modified=1;
    }
    if(data & ~*offset) {
        flashFlags.zeroto1=1;
    }
    *offset=data;
}

/**
 * Write one byte and flush to flash.
 * @param flashAddr the destination address of the byte to be written
 * @param flashData the data byte to be written
 */
void writeFlashByte( BYTE * flashAddr, BYTE flashData ) {
    writeFlashImage( flashAddr, flashData );    // Put data into memory image, if necessary flush image to flash first
    flushFlashImage();                          // Flush any changes
}

/**
 * Write one word to the flash buffer.
 * @param flashAddr the destination address of the byte to be written
 * @param flashData the data word to be written
 */
void setFlashWord( WORD * flashAddr, WORD flashData ) {
     writeFlashImage( (BYTE*)flashAddr, (BYTE)(flashData & 0x00FF) );    // Put LS byte into memory image, if necessary flush image to flash first
     writeFlashImage( ((BYTE*)flashAddr)+1, (BYTE)(flashData >> 8) );      // Repeat for MSByte
}

/**
 * Write a range of addresses to Flash.
 * @param flashAddr
 * @param bufferaddr
 * @param bufferSize
 */
void setFlashBuffer( BYTE * flashAddr, BYTE *bufferaddr, BYTE bufferSize ) {
    BYTE    i;
    for ( i=0; i<bufferSize; i++) {
        writeFlashImage( flashAddr+i, bufferaddr[i] );
    }
}

// *************** EEPROM operations

/**
 * Read a byte from data EEPROM.
 * @param addr the address to be read
 * @return the byte from EEPROM
 */
BYTE ee_read(WORD addr) {
    while (EECON1bits.WR)       // Errata says this is required
        ;
    // EEADRH = addr >> 8;        //  High byte of address to read
    SET_EADDRH(addr >> 8);
    EEADR = addr & 0xFF;       	/* Low byte of Data Memory Address to read */
    EECON1bits.EEPGD = 0;    	/* Point to DATA memory */
    EECON1bits.CFGS = 0;    	/* Access program FLASH/Data EEPROM memory */
    EECON1bits.RD = 1;			/* EEPROM Read */
    while (EECON1bits.RD)
        ;
#ifdef __XC8
    asm("NOP");                 /* data available after a NOP */
    asm("NOP");                 /* data available after a NOP */
    asm("NOP");                 /* data available after a NOP */
    asm("NOP");                 /* data available after a NOP */
    asm("NOP");                 /* data available after a NOP */
    asm("NOP");                 /* data available after a NOP */
#else
     _asm
        nop
    _endasm
#endif
    return EEDATA;
}

/**
 * Write one byte to data EEPROM.
 * @param addr the address to be written
 * @param data the data to be written
 */
void ee_write(WORD addr, BYTE data) {
    SET_EADDRH(addr >> 8);      // High byte of address to write
    EEADR = addr & 0xFF;       	/* Low byte of Data Memory Address to write */
    EEDATA = data;
    EECON1bits.EEPGD = 0;       /* Point to DATA memory */
    EECON1bits.CFGS = 0;        /* Access program FLASH/Data EEPROM memory */
    EECON1bits.WREN = 1;        /* Enable writes */
    di();         /* Disable Interrupts */
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
#ifdef __XC8__
    while (EECON1bits.WR)
        ;
    //asm("BTFSC EECON1, 1");                 // should wait until WR clears
#else
    _asm nop
         nop _endasm
#endif
    ei();         /* Enable Interrupts */
    while (!EEIF)
        ;
    EEIF = 0;
    EECON1bits.WREN = 0;		/* Disable writes */
}

/**
 * Read a WORD (16 bit) word from EEPROM.
 * Data is stored in little endian format
 * @param addr the address to be read
 * @return the WORD from EEPROM
 */
WORD ee_read_short(WORD addr) {
	WORD ee_addr = addr;
    WORD ret = ee_read(ee_addr++);
    
	ret = ret | ((WORD)ee_read(ee_addr) << 8);
	return ret;
}

/**
 * Write a WORD (16 bit) data to EEPROM.
 * Data is stored in little endian format.
 * @param addr the address to be written
 * @param data the data to be written
 */
void ee_write_short(WORD addr, WORD data) {
	WORD ee_addr = addr;
	ee_write(ee_addr++, (BYTE)data);
	ee_write(ee_addr, (BYTE)(data>>8));
}

/**
 * Read the DevId from the Config area
 */
#ifdef __XC8__
extern const WORD devId @0x3FFFFE;
#endif
WORD readCPUType( void ) {
#ifdef __XC8__
    return devId;
#else
    WORD id = *(far rom WORD*)0x3FFFFE;
    return( id );
#endif
    
/*
        TBLPTR = 0xFFFE;
        TBLPTRU = 0x3F;
        EECON1=0X80;
        di();
_asm
        TBLRDPOSTINC
_endasm
        b1 = TBLAT;
_asm
        TBLRDPOSTINC
_endasm
        b2 = TBLAT;
        ei();
        return (WORD)b2<<8 | b1;
 */
}
