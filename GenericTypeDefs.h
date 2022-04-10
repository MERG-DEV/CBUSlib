/* 
 * File:   GenericTypeDefs.h
 * Author: Ian
 *
 * Created on 15 April 2017, 21:13
 */

#ifndef GENERICTYPEDEFS_H
#define	GENERICTYPEDEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

    /*
     * Whereas the C18 compiler seemed to have these the XC8 does not.
     * When compiling with C18 make sure you use the standard GenericTypes.h
     * instead of this one.
     */
    typedef unsigned char BYTE;
    typedef unsigned int WORD;
    typedef unsigned char BOOL;
    typedef unsigned long DWORD;
    
    #define TRUE 1
    #define FALSE 0

#ifdef	__cplusplus
}
#endif

#endif	/* GENERICTYPEDEFS_H */
