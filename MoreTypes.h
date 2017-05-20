/* 
 * File:   MoreTypes.h
 * Author: Pete Brownlow
 *
 * Created on 18 March 2015, 13:23
 *
 * Additional useful types and macros
 *
 */

// ******************************************************************************************************
//   This work is licensed under the:
//      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//   To view a copy of this license, visit:
//      http://creativecommons.org/licenses/by-nc-sa/4.0/
//   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
//
//   License summary:
//    You are free to:
//      Share, copy and redistribute the material in any medium or format
//      Adapt, remix, transform, and build upon the material
//
//    The licensor cannot revoke these freedoms as long as you follow the license terms.
//
//    Attribution : You must give appropriate credit, provide a link to the license,
//                   and indicate if changes were made. You may do so in any reasonable manner,
//                   but not in any way that suggests the licensor endorses you or your use.
//
//    NonCommercial : You may not use the material for commercial purposes under this license. **(see note below)
//
//    ShareAlike : If you remix, transform, or build upon the material, you must distribute
//                  your contributions under the same license as the original.
//
//    No additional restrictions : You may not apply legal terms or technological measures that
//                                  legally restrict others from doing anything the license permits.
//
//   ** Commercial use is possible - for commercial use, please contact the orignal copyright holder(s) to agree licensing terms
// ******************************************************************************************************



#ifndef MORETYPES_H
#define	MORETYPES_H

#ifdef	__cplusplus
extern "C" {
#endif

// Macros for access to arrays of bits
// Define the array as an array of BYTE, length should be the number of bits you want divided by 8 (rounded up)

#define arrayTestBit( array, index ) \
  ( array[index>>3] & ( 1<<(index & 0x07) ) )

#define arraySetBit( array, index ) \
  ( array[index>>3] |= ( 1<<(index & 0x07) ) )

#define arrayClearBit( array, index ) \
  ( array[index>>3] &= ~( 1<<(index & 0x07) ) )


#ifdef	__cplusplus
}
#endif

#endif	/* MORETYPES_H */

