#ifndef __FLiM_H
#define __FLiM_H

/*

 Copyright (C) Pete Brownlow 2011-2017   software@upsys.co.uk

 FLiM.h - Definitions for CBUS FLiM module - Part of CBUS libraries for PIC 18F
 
 	The FLiM routines have no code or definitions that are specific to any
	module, so they can be used to provide FLiM facilities for any CBUS module
	using these libraries.

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

 Revision history for this file:
  	02/06/11    PNB	- Define data structures and function prototypes
 	09/05/11	PNB	- Initial outline
                PNB - Develop data structures for node variable and events tables
    30/12/16    PNB - Add new test mode definitions
*/

#include "cbus.h"
#include "events.h"
#include "romops.h"
#include "EEPROM.h"

#define EVT_NUM		255 // Number of events in event table
#define EVperEVT	10  // Event variables per event

#define FLiM_DEBOUNCE_TIME HUNDRED_MILI_SECOND
#define FLiM_HOLD_TIME  4 * ONE_SECOND
#define SET_TEST_MODE_TIME 8 * ONE_SECOND
#define NEXT_TEST_TIME ONE_SECOND


// The parameter bytes can be accessed either as an array of bytes or as a structure with each
// byte name for its meaning, as defined in this union.

// These routines have no knowledge of where the parameter bytes are stored, they are always 
// passed as a parameter.

typedef struct
{
    BYTE	manufacturer;
    BYTE	minor_ver;
    BYTE	module_id;
    BYTE	number_of_events;
    BYTE	evs_per_event;
    BYTE	number_of_nvs;
    BYTE	major_ver;
    BYTE    module_flags;
    BYTE    cpu_id;
    BYTE    bus_type;
    DWORD   load_address;
    DWORD   cpumid;
    BYTE    cpuman;
    BYTE    beta;
} ParamVals;

typedef	union
{
    ParamVals   params;
    BYTE        bytes[sizeof(ParamVals)];
} FLiMParameters;

typedef rom ParamVals       *prmvalptr;
typedef rom FLiMParameters  *FLiMprmptr;


typedef BYTE    SpareParams[4];

typedef struct
{
    WORD        parameter_count;
    DWORD       module_type_name;
    WORD        parameter_checksum;
} FCUParams;

typedef struct
{
    ParamVals   vals;
    SpareParams spares;
    FCUParams   FCUparams;
} ParamBlock;


// NODE VARIABLES
//
// For node variables, the application code will define a union of the form:
//
//	Typedef 
//		union
//		{
//			NodeBytes	nodevars[NUMBER_OF_NVS];
//			ModuleNodeDefs	module_nodevars;
//		} NodevarTable
//
//  Where ModuleNodeDefs is a structure type that defines the meaning and structure
//  of each node variable as used by that application.
//
//  NVPtr is then defined to point to the NodeBytes member of the union, thus giving the
//  generic FLiM code access to the node variables table without requiring any knowledge
//  of the structure of the node variables themselves. 


typedef	BYTE		NodeBytes[];


// EVENTS
//
// The events are stored as a hash table in flash (flash is faster to read than EEPROM)
// There can be up to 255 events. Since the address in the hash table will be 16 bits, and the
// address of the whole event table can also be covered by a 16 bit address, there is no
// advantage in having a separate hashed index table pointing to the actual event table.
// Therefore the hashing algorithm produces the index into the actual event table, which
// can be shifted to give the address - each event table entry is 16 bytes. After the event
// number and hash table overhead, this allows up to 10 EVs per event.
//
//  For the event table, the application code will define a structure of the form:
//  typedef	struct
//  {
//      EventEntry      event;          // Event id and hash table management, defined in FLiM module
//      ModuleEvs       modEvs;         // Application specific EV definitions
//  } ; ModuleEventEntry;
//
// Where ModuleEvs defines the specific EV usage by that application
// ModuleEvs may itself be a further combination of structures and/or unions depending on the EV usage by that code,
// the generic FLiM code needs no knowledge of specific EV usage
//
//  EVPtr is then defined to point to an EventTable Entry structure, which mirrors the application specific definition without the EV specific usage information.
//  thus giving the generic FLiM code access to the event table, and event hash table information, without requiring any knowledge
//  of the structure of the event variables themselves.
//  The application code can use the same pointer, type transferred to its own modEVptr, to access application specific information in the event table.

typedef union
{
    struct
    {
        BOOL    conctinues:1;   // Continues in next linked entry
        BOOL    continuation:1; // Continuation of previous event entry
        BOOL    freeEntry:1;
        BOOL    producerEvent:1;     // Set for producer event
        BOOL    SoDEvent;           // Set if this is a SoD event
    };
    BYTE    free;       // Set to 0xFF for free entry, initially set to zero for entry in use, then producer flag set if required.
} EventTableFlags;

typedef struct
{
    EventTableFlags     evtFlags;      // Set to 0xFF for a free entry
    BYTE                nextEvent;     // Event index to Link to next entry on the same hash value. Zero terminated list - linked event cannot be at index 0
    WORD                node_id;
    WORD                evt_id;
} EventEntry;

typedef
    BYTE                evBytes[EVperEVT];    // 10 bytes for EVs in each event table entry


typedef struct
{
    EventEntry          event;
    evBytes             ev;
} EventTableEntry;


typedef EventTableEntry EvTable[];


// State machine for FliM operations

enum FLiMStates
{
    fsSLiM=0,		// In SLiM, button not pressed
    fsFLiM,		// In FLiM mode
    fsPressed,          // Button pressed, waiting for long enough
    fsFlashing,         // Button pressed long enough,flashing now
    fsFLiMSetup,        // In FLiM setup mode (LED flashing)
    fsFLiMRelease,      // Release FLiM back to SLiM
    fsSetupDone,	// Exiting setup mode
    fsFLiMLearn,        // In FLiM learn mode
    fsPressedFLiM,      // Pressed whilst in FLiM mode
    fsPressedSetup,     // Pressed during setup
    fsPressedTest,      // Pressed during test
    fsTestMode,         // Self test mode
    fsNextTest,         // Move on to next test
    fsTestInput,        // Input to current test mode
    fsNoData = 0xFF     // Uninitialised memory indicates no FLiM data present
};

// External variables for other modules to access FLiM

extern rom	NodeBytes 	*NVPtr;     // pointer to Node variables table.  \_ These can be array as defined here or with specific structures in module specific code
extern rom	EventTableEntry	*EVTPtr;    // pointer to Event variables table. /


extern BOOL	NV_changed;	
extern enum     FLiMStates flimState;

// Data stored in program ROM - FLiM parameters. Node Variables and Event Variables storage definition is in the application module.

extern const rom ParamVals  FLiMparams;
extern const rom FCUParams  FCUparams;
extern const rom char       module_type[];


// Function prototypes for FLiM operation

void 	FLiMInit( WORD defaultNodeID );

// Check FLiM pushbutton status

void FLiMSWCheck( void );


// Enter FLiM mode - called when module wants to be allocated a node number for FLiM
//	(for example, when pusbbutton pressed.)
// Returns the node number allocated

void requestNodeNumber( void );


// Revert to SLiM mode - called when module wants to go back to slim
// for example when pushbutton pressed whilst in FLiM mode

void	SLiMRevert(void);

// parse incoming message for events or commands

BOOL parseCBUSMsg(BYTE *msg);

// Process FLiM opcode - called when any module specific CBUS opcodes have 
// been dealt with.  Returns true if the opcode was processed.

BOOL 	parseFLiMCmd(BYTE *rx_ptr);



// Internal functions

void    QNNrespond( void );
void 	doNNack( void );
void	doRqnpn(BYTE idx);
void 	doNnclr(void);
void 	doNvrd(BYTE NVindex);
void	doNvset(BYTE NVindex, BYTE NVvalue);
void 	doRqnp(void);
void    doRqmn(void);
void 	doSnn( BYTE *rx_ptr );
void	doError(unsigned int code);
BOOL	thisNN( BYTE *rx_ptr);
void    sendStartupSod(WORD defaultEvent);
void    SaveNodeDetails(WORD Node_id, enum FLiMStates flimState);
WORD    readCPUType( void );



#endif	// __FLiM_H
