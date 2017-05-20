#ifndef __SPI_H
#define	__SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 spi.h - Definitions for PIC SPI interface - part of CBUS libraries for PIC 18F

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
// Revision History
//
// 25/01/14     1.0     PNB Coding started/*



#include <devincs.h>
    
// SPI I/O pin definitions

#define SCK_TRIS            (TRISCbits.TRISC3)
#define SDI_TRIS            (TRISCbits.TRISC4)
#define SDO_TRIS            (TRISCbits.TRISC5)


// SPI register bit definitions

#define SPI_DONEFLAG         (PIR1bits.SSPIF)

// Values for SSPCON1 register to set SPI master mode at various speeds

#define SPI_MASTER_FOSCd4       0
#define SPI_MASTER_FOSCd8       0x0A
#define SPI_MASTER_FOSCd16      1
#define SPI_MASTER_FOSCd64      2
#define SPI_MASTER_TMR2d2       3

#define SPI_CKP_LOW             0   // Clock idle state
#define SPI_CKP_HIGH            8
    


// Macros for SPI operation


#if defined (__18CXX)
    #define ClearSPIDoneFlag()  {SPI_DONEFLAG = 0;}
    #define WaitForDataByte()   {while(!SPI_DONEFLAG); SPI_DONEFLAG = 0;}
    #define SPI_ON_BIT          (SSPCON1bits.SSPEN)
#elif defined(__C30__)
    #define ClearSPIDoneFlag()
    static inline __attribute__((__always_inline__)) void WaitForDataByte( void )
    {
        while ((SSPSTATbits.SPITBF == 1) || (SSPSTATbits.SPIRBF == 0));
    }

    #define SPI_ON_BIT          (SSPSTATbits.SPIEN)
#elif defined( __PIC32MX__ )
    #define ClearSPIDoneFlag()
    static inline __attribute__((__always_inline__)) void WaitForDataByte( void )
    {
        while (!SSPSTATbits.SPITBE || !SSPSTATbits.SPIRBF);
    }

    #define SPI_ON_BIT          (SSPCON1bits.ON)
#else
    #error Determine SPI flag mechanism
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

