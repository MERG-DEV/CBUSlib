#ifndef __STATUSLEDS_H
#define __STATUSLEDS_H

/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

CBUS Status LEDs - Definitions for the SLiM and FLiM status LEDs

 This code is for a standard CBUS module that has a yellow and green LED
 

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

#include "GenericTypeDefs.h"
#include "TickTime.h"
#include "hwsettings.h"

#define SLOW_FLASH_TIME HALF_SECOND     // On time for a slow flash
#define FAST_FLASH_TIME HALF_SECOND/3   // On time for a fast flash - 3 times as fast as slow flash

enum FlashStates 
{
    flNone=0,	// No flashing
    flFLiMSlow,	// SLow flash of FLiM LED
    flFLiMFast  // Fast flash of FLiM LED
};

#define SHORT_FLICKER_TIME  HUNDRED_MILI_SECOND     // a short flicker
#define LONG_FLICKER_TIME   HALF_SECOND             // a long flicker
enum FlickerStates 
{
    flickOff=0,	// Flicker LED off
    flickWaitingOn,	// waiting to flick on at next poll
    flickOn         // on and counting down
};

void initStatusLeds(void);
void setSLiMLed(void);
void setFLiMLed(void);
void setStatusLed( BOOL FLiMLED );
void startFLiMFlash( BOOL fast );
void doFLiMFlash(void);
void checkFlashing(void);


#endif	// __STATUSLEDS_H
