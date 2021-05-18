#ifndef __EEPROM_H
#define __EEPROM_H

/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

EEPROM.h - Definitions for EEPROM usage - part of CBUS libraries for PIC 18F

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

// CBUS EEPROM usage is at the top of EEPROM, which leaves space from 0 upwards for use by the MLA.
// Note that if you are using internal EEPROM for MLA, you must reduce the amount of EEPROM declared to MLA by the amount used here by CBUS

#ifdef EE256
    #define EE_TOP  0xFF
    #define EE_BOTTOM 0x00
    #define	SET_EADDRH(val)             	// EEPROM high address reg not present in 2480/2580, so just do nothing
#endif

#ifdef EE1024
    #define EE_TOP  0x3FF
    #define EE_BOTTOM 0x00
    #define	SET_EADDRH(val) EEADRH = val	// EEPROM high address is present, so write value
#endif

// Top bytes for Bootloader and CBUS library usage - these values are address offsets into EEPROM
// Library uses top 8 for general storage then next 32 for event status
// application starts at EE_TOP-40

#define EE_BOOT_FLAG        ((BYTE*)(EE_TOP))       // Set to FF to enter bootloader
#define EE_CAN_ID           ((BYTE*)(EE_TOP-1))     // 7 bit CANID 1 to 127
#define EE_NODE_ID          ((WORD*)(EE_TOP-3))     // 16 bit value for node number. Size = 2
#define EE_FLIM_MODE        ((BYTE*)(EE_TOP-4))     // Enumerated value for SLiM/FLiM mode
#define EE_VERSION          ((BYTE*)(EE_TOP-5))     // Indicates if EEPROM needs initialising or upgrading
#ifdef AREQ_SUPPORT
#define EE_AREQ_STATUS      ((BYTE*)(EE_TOP-39))    // Event status bits. Size=34
#define EE_APPLICATION      ((BYTE*)(EE_TOP-40))
#else
#define EE_APPLICATION      ((BYTE*)(EE_TOP-40))    // this could be moved but better for compatability if stays in same location
#endif

#endif	// __EEPROM_H
