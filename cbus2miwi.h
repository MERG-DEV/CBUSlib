/*
 Copyright (C)2014 Pete Brownlow

 Wrapper layer from CBUS to MiWi routines

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 
 */


#ifndef __CBUS2MIWI_H
#define __CBUS2MIWI_H

#if !defined(CBUS_OVER_CAN)

// If there is no CAN interface, defined the CBUS buffer as just the data bytes

enum CbusBytes {
        d0,
        d1,
        d2,
        d3,
        d4,
        d5,
        d6,
        d7
};

#endif

#endif

