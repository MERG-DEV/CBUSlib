/*
 Routines for CBUS Happenings and Actions event operations - part of CBUS libraries for PIC 18F
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
**************************************************************************************************************
	The FLiM routines have no code or definitions that are specific to any
	module, so they can be used to provide FLiM facilities for any module 
	using these libraries.
	
*/ 
/* 
 * File:   happeningsActions.h
 * Author: Ian Hogg
 * 
 * This file contains the structure definition for a pin configuration. It 
 * provides the mapping between IO, Pin, Port/bit no.
 *
 * Created on 03 June 2021, 19:13
 */

#ifndef HAPPENINGSACTIONS_H
#define	HAPPENINGSACTIONS_H
#ifdef	__cplusplus
extern "C" {
#endif

 // function definitions
extern void deleteActionRange(ACTION_T action, unsigned char number);
extern void deleteHappeningRange(HAPPENING_T action, unsigned char number);
extern BOOL getProducedEvent(HAPPENING_T action);
extern BOOL getDefaultProducedEvent(HAPPENING_T paction);
extern BOOL sendProducedEvent(HAPPENING_T paction, BOOL on);
#ifdef AREQ_SUPPORT
void    doAreq(WORD nodeNumber, WORD eventNumber);
#endif

#ifdef PRODUCED_EVENTS
// the lookup table to find an EventTable entry by Happening
extern BYTE happening2Event[NUM_HAPPENINGS];    // MIO: 64+8 bytes
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* HAPPENINGSACTIONS_H */

