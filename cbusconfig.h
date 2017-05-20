#ifndef __CBUSCONFIG_H
#define __CBUSCONFIG_H

/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

 cbusconfig.h - CBUS transport configuration settings
                Part of CBUS libraries for PIC 18F

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

// If your module has just one CAN interface (as is the case with most CBUS modules)
// then no change is required here, as that is the default condition.

// If your module uses a different transport, or has more than one CBUS interface, and is not in the list below,
// then add a section here for your module with the module name defined in the project file.

#if defined(CANWI)
    #define CBUS_OVER_CAN    0
    #define CBUS_OVER_MIWI   1
#elif defined(WICAB)
    #define CBUS_OVER_MIWI   0
#elif defined(CANEther)
    #define CBUS_OVER_CAN    0
    #define CBUS_OVER_TCP    1
#else
    #define CBUS_OVER_CAN    0
#endif

#endif