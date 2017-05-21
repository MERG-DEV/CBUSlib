/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

  CBUS CANPanel - PIC config and clock management for CANPanel module

 This code is for a CANPanel CBUS module, to control up to 64 LEDs (or 8 x 7 segment displays)
 and up to 64 push buttons or on/off switches

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
	
 For version number and revision history see CANPanel.h

*/
/*
 * Chip specific operations.
 */
#include "hwsettings.h"

    // The config values are pasted in to this source file after using the <Window->PIC memory 
    // views->Configuration Bits> menu entry in MPLABX
    // 16MHz HS1 and x4 PLL

#ifdef CPUF18K
    #pragma config FOSC=HS1, PLLCFG=ON, FCMEN=OFF, IESO=OFF, SOSCSEL = DIG   
    #pragma config PWRTEN=OFF, BOREN=OFF, BORV=3, WDTEN = OFF, WDTPS=256
    #pragma config MCLRE=ON, CANMX=PORTB
    #pragma config XINST=OFF, BBSIZ=BB1K, STVREN=OFF
    #pragma config CP0=OFF, CP1=OFF, CP2=OFF, CP3=OFF, CPB=OFF, CPD=OFF
    #pragma config WRT0=OFF, WRT1=OFF, WRT2=OFF, WRT3=OFF, WRTB=OFF, WRTC=OFF, WRTD=OFF
    #pragma config EBTR0=OFF, EBTR1=OFF, EBTR2=OFF, EBTR3=OFF, EBTRB=OFF
#elif defined CPUF18F
    #define BORSTAT BOHW
    #pragma config OSC=HS, FCMEN=OFF, IESO=OFF
    #pragma config PWRT=ON, BOREN=BORSTAT, BORV=3, WDT = OFF, WDTPS=256
    #pragma config MCLRE=ON, LPT1OSC=OFF, PBADEN=OFF, DEBUG=OFF
    #pragma config XINST=OFF, BBSIZ=1024, LVP=OFF, STVREN=OFF
    #pragma config CP0=OFF, CP1=OFF, CPB=OFF, CPD=OFF
    #pragma config WRT0=OFF, WRT1=OFF, WRTB=OFF, WRTC=OFF, WRTD=OFF
    #pragma config EBTR0=OFF, EBTR1=OFF, EBTRB=OFF
#endif

#ifndef __XC8__
#pragma udata MAIN_VARS
#endif

BYTE clkMHz;        // Derived or set system clock frequency in MHz



// Set system clock frequency variable - either derived from CAN baud rate register
// if set by bootloader, or default value for CPU type if not

void setclkMHz( void )
{
    
    #ifdef BOOTLOADER_ABSENT
        #if defined(CPUF18F)
            clkMHz = 16;
        #else
            clkMHz= 64;
        #endif    
    #else    
        #ifdef __C32__
            clkMHz = (( CiCFG & 0x3F ) + 1 ) << 2;        // Convert CAN config register into clock MHz
        #else 
            clkMHz = (( BRGCON1 & 0x3F ) + 1 ) << 2;      // Convert BRGCON1 value into clock MHz.
        #endif
    #endif    

}

  
