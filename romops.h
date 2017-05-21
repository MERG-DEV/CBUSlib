#ifndef __ROMOPS_H
#define __ROMOPS_H

/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 romops.h - Definitions for EEPROM and Flash routines - part of CBUS libraries for PIC 18F

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

#include "devincs.h"
#include "GenericTypeDefs.h"

// Bit definitions for EEPROM flags

#ifdef CPUF18K
    #define EEIF PIR4bits.EEIF
#endif

#ifdef CPUF18F
    #define EEIF PIR2bits.EEIF
#endif



// Definitions for inline assembler
#ifdef __C18__
#define W   0
#define F   1
#endif 

// Structure for tracking Flash operations

typedef union
{
 BYTE asByte;       
 struct
 {
  BYTE valid:3;    //must be 101 (5) to be valid
  BYTE loaded:1;   // if buffer is loaded
  BYTE modified:1; //flag if buffer is modified
  BYTE zeroto1:1;  //flag if long write with block erase
 };
} FlashFlags;


// extern rom BYTE bootflag;


void initRomOps(void);
void writeFlashByte( BYTE * flashAddr, BYTE flashData );
void writeFlashImage(BYTE * addr, BYTE data);
#define setFlashByte( a, b )    writeFlashImage( a, b)
void setFlashWord( WORD * flashAddr, WORD flashData );
void setFlashBuffer( BYTE * flashAddr, BYTE *bufferaddr, BYTE bufferSize );
void flushFlashImage( void );;
BYTE readFlashBlock(WORD flashAddr);

BYTE ee_read(WORD addr);
void ee_write(WORD addr, BYTE data);
WORD ee_read_short(WORD addr);
void ee_write_short(WORD addr, WORD data);



#endif	// __ROMOPS_H
