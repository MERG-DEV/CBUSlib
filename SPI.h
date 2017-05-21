//
// MERG CBUS common libraries
//
// SPI interface header file

//      All source code is copyright (C) the author(s) concerned
//      (C) 2014 Pete Brownlow      merg@uspys.co.uk
//
//   This program is provided as free software for non-commercial use.
//   For non-commercial use, you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, version 3 of the License, as set out
//   at <http://www.gnu.org/licenses/>.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//   For commercial use, please contact the author(s) in order to agree terms
//
//   As set out in the GNU General Public License, you must retain and acknowledge
//   the statements above relating to copyright and licensing. You must also
//   state clearly any modifications made.  Please therefore retain this header
//   and add documentation of any changes you make. If you distribute a changed
//   version, you must make those changes publicly available.
//
//   The GNU license requires that if you distribute this software, changed or
//   unchanged, or software which includes code from this software, including
//   the supply of hardware that incorporates this software, you MUST either
//   include the source code or a link to a location where you make the source
//   publicly available. The best way to make your changes publicly available is
//   via the MERG online resources.  See <www.merg.org.uk>
//
//
//  Note:   This source code has been written using a tab stop and indentation setting
//          of 4 characters. To see everyting lined up correctly, please set your
//          IDE or text editor to the same settings.
//
// Revision History
//
// 25/01/14     1.0     PNB Coding started/*

#include "devincs.h"

#ifndef __SPI_H
#define	__SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

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

//#ifdef CPUF18F
//    #define GetSystemClock()               (16000000ul)
//#else
//    #define GetSystemClock()               (64000000ul)
//#endif
//
//#define GetInstructionClock()          (GetSystemClock()/4)
//#define GetPeripheralClock()           (GetSystemClock()/4)    // For PIC18 there are 4 clock cycles per instruction
//
//#define GetSystemMHz()                 (GetSystemClock()/1000000)
//#define GetInstructionMHz()            (GetInstructionClock()/1000000)


#if defined (__18CXX) || defined(CPUF18K)
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

