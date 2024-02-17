/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 devincs.h - PIC processor and family related definitions - part of CBUS libraries for PIC 18F

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

#ifndef DEVINCS_H
#define	DEVINCS_H

#ifdef	__cplusplus
extern "C" {
#endif

// Include the appropriate definition file for the processor that has been selected for this project
#ifdef __XC8
#include <xc.h>
#else
#include <p18cxxx.h>
#endif
    
// Set CPUF to the PIC18 series family, so that we can do conditional assembly for the k series and original PIC18 processors
// This is defined only for the CAN processors at present
          
#if defined(__18F25K80) || defined(__18F26K80) || defined(__18F45K80) || defined(__18F46K80)|| defined(__18F65K80) || defined(__18F66K80)
    #define CPUF18K
#else
    #define CPUF18F
#endif

// Define 26K series family for amount of flash available
    
#if defined (__18F26K80) || defined(__18F46K80) || defined(__18F66K80)   
    #define CPUF26K
#endif
    
// Define the amount of EEPROM available
    
#if defined(__18F2480) || defined(__18F2580)
    #define EE256
#else
    #define EE1024
#endif

    
// Set CPU code for CBUS parameter block

#if defined(__18F25K80)
    #define CPU P18F25K80
#endif

#if defined(__18F26K80)
    #define CPU P18F26K80
#endif

#if defined(__18F46K80)
    #define CPU P18F46K80
#endif

#if defined(__18F2580)
    #define CPU P18F2580
#endif

#if defined(__18F2585)
    #define CPU P18F2585
#endif

#if defined(__18F2680)
    #define CPU P18F2680
#endif

    


#ifdef	__cplusplus
}
#endif

#endif	/* DEVINCS_H */

