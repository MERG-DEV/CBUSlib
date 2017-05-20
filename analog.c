/*
 Copyright (C) Pete Brownlow 2014

 Analogue I/O routines

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "analog.h"


#pragma code APP


#include <compiler.h>
#include <hardwareprofile.h>


BYTE    AtoDValue;
BOOL    AtoDStarted;


void initAnalog(void)
{
    ADCON0 = ADCHAN_SEL + 1;    // Select channel and enable A-D
    ADCON1 = 0;                 // Use supply rails as references
    ADCON2 = ADCON2_INIT;
    AD_TRIS = 1;                // Set analogue port as input
    ANSEL = AD_SEL;             // Enable analogue input pin
    ANSELH = 0;

    AtoDValue = 0;
    AtoDStarted = FALSE;
}

BYTE updateAnalog( void )

// This routine is called regularly from main loop or ISR to update the A-D value.
// It alternately initiates a conversion and reads the result (if ready)
// The returned value is a running average of the conversions
{
    if (!ADCON0bits.GO)
    {
        if (AtoDStarted)
        {
            AtoDValue += (ADRESH / 2);      // Update with new conversion result, 0-127 range
            AtoDValue /= 2;                 // Keep running average, 0-127 range
            AtoDStarted = FALSE;
        }
        else // start a conversion
        {
            ADCON0bits.GO = 1;
            AtoDStarted = TRUE;
        }
    }
    return( AtoDValue );
}

