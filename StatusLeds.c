/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 CBUS Status LEDs - I/O routines for the SLiM and FLiM status LEDs

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

 Modifications to initStatusLeds() by Ian Hogg 23/5/2017

*/
#include "module.h"
#include "StatusLeds.h"


TickValue           flashTime;
enum FlashStates    flashState; 

//******* Routines to set the Green and Yellow SLiM and FLiM LEDs *************************

/**
 * Initialise with both LEDs on.
 */
void initStatusLeds(void) 
{
    TRIS_LED1Y = 0;
    TRIS_LED2G = 0;
    LED1Y = 0;
    LED2G = 0;
    flashState = flNone;
}
/**
 * Set the Green LED and turn of the Yellow LED.
 */
void setSLiMLed(void) 
{
    LED1Y = 0;
    LED2G = 1;
    flashState = flNone;
}
/**
 * Set the yellow LED and turn off the green LED.
 */
void setFLiMLed(void) 
{
    LED1Y = 1;
    LED2G = 0;
    flashState = flNone;
}

/**
 * Sets either the FLiM or the SLiM LED.
 * @param FLiMLED if true turns on the FLiM LED otherwise turn on the SLiM LED.
 */
void setStatusLed( BOOL FLiMLED ) 
{
    if (FLiMLED)
        setFLiMLed();
    else
        setSLiMLed();
}

/**
 * Flash the FLiM LED.
 */
void doFLiMFlash(void) 
{
    LED1Y = !LED1Y;
    flashTime.Val = tickGet();
}

/**
 * Start the FLiM LED flashing at the specified rate.
 * @param fast if true flash the LED quickly
 */
void startFLiMFlash( BOOL fast ) 
{
    flashState = (fast? flFLiMFast : flFLiMSlow);
    doFLiMFlash();
}

/**
 * Process the FLiM LED flashing. Call regularly.
 */
void checkFlashing(void) 
{
    switch (flashState) 
    {
        case flFLiMSlow:
            if (tickTimeSince(flashTime) > SLOW_FLASH_TIME)
                doFLiMFlash();
            break;
            
        case flFLiMFast:
            if (tickTimeSince(flashTime) > FAST_FLASH_TIME)
                doFLiMFlash();
            break;
    }        
}

