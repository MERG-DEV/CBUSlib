/*
 Copyright (C) Pete Brownlow 2014-2015   software@upsys.co.uk

 Routines for CBUS FLiM operations - part of CBUS libraries for PIC 18F

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
	
	9/5/11	Pete Brownlow	- Initial coding

 */

#include <string.h>
#include "FLiM.h"
#include "module.h"
#include "StatusLeds.h"
#include "events.h"
#include "romops.h"

extern BOOL validateNV(BYTE NVindex, BYTE oldValue, BYTE newValue);
extern void actUponNVchange(BYTE NVindex, BYTE NVvalue);


#ifdef __XC8__
WORD    deviceid;      // Device id in config memory
// Static variables local to this library
#else
#pragma romdata CPUID
WORD    deviceid;      // Device id in config memory
#pragma romdata


// Static variables local to this library

#pragma udata MAIN_VARS
#endif

enum FLiMStates flimState;          // This is stored in EEPROM with node id
enum FLiMStates prevFlimState;      // Store previous state in setup mode
TickValue   switchTime;             // Debouncing FLiM switch

#ifdef __XC8__
NodeBytes 	*NVPtr;     // Node variables in ROM
//EventTableEntry     *EVTPtr;    // Event table in ROM
#else
rom	NodeBytes 	*NVPtr;     // Node variables in ROM
rom EventTableEntry     *EVTPtr;    // Event table in ROM
   
#pragma code APP
#pragma udata
#endif

BOOL	FLiMFlash;              // LED is flashing
BOOL	FlashStatus;			// Control flash on/off of LED during FLiM setup etc
BOOL    NV_changed;
/**
 * FLimInit called during initialisation Initialises FLiM support which will 
 * also include support for events in SLiM and CBUS/CAN
 *  
 */
void flimInit(void) {
    initRomOps();    
    flimState = ee_read((WORD)EE_FLIM_MODE);   // Get flim mode from EEPROM
    prevFlimState = flimState;
    eventsInit();
    cbusInit(DEFAULT_NN);
    FlashStatus = FALSE;

    // Initialise node variables

//	NVPtr = (NodeBytes*)nodeVarTable.nodevars;         // Node Variables table
//    NV = (rom ModuleNvDefs*) NVPtr;
//	EVTPtr = eventTable;           // Event table
	NV_changed = FALSE; 
} // flimInit



/**
 * Check for FLiM button pressed and control entry into FLiM setup mode.
 * The definition of FLiM_SW in hwsettings.h sets the I/O pin for the FLiM switch.
 * A module that uses a mechanism for FLiM entry other than a simple push button
 * can define FLiM_SW as a macro or procedure call as required.
 */
void FLiMSWCheck( void ) {
    switch (flimState)
    {
       case fsFLiM:
       case fsSLiM:
            if (!FLiM_SW) {
                prevFlimState = flimState;
                flimState = fsPressed;
                switchTime.Val = tickGet();
            }
            break;
        case fsPressed:
            if (!FLiM_SW) {
                if (tickTimeSince(switchTime) > FLiM_HOLD_TIME) {
                    flimState = fsFlashing;   // Held long enough so start flashing LED
                    startFLiMFlash(FALSE);    // slow flash for FLiM setup
                }
            } else if ((prevFlimState == fsFLiM) && (tickTimeSince(switchTime) > FLiM_DEBOUNCE_TIME)) {
                // short press and release in FLiM
                flimState = fsFLiMSetup;
                requestNodeNumber();
            } else {
                flimState = prevFlimState; // But not held down long enough, revert to previous state
            }
            break;
        case fsFlashing:
            if (FLiM_SW) {
                // Button released
                if (prevFlimState == fsSLiM) {
                    flimState = fsFLiMSetup;
                    requestNodeNumber();
                } else {
                    flimState = fsSLiM;
                    SLiMRevert();
                    setSLiMLed();
                }
            } else {
                if (tickTimeSince(switchTime) > SET_TEST_MODE_TIME) {
                    flimState = fsPressedTest;
                    switchTime.Val = tickGet();
                    startFLiMFlash(TRUE);       // Fast flash for test mode
                }
            }
            break;
        case fsFLiMSetup:  // Button pressed whilst in setup mode
            if (!FLiM_SW) {
                flimState = fsPressedSetup; // Wait for debounce
                switchTime.Val = tickGet();
            }
            break;
        case fsPressedSetup:   // Button was pressed whilst in setup mode
            if (FLiM_SW) {
                if ((tickTimeSince(switchTime) > FLiM_DEBOUNCE_TIME)) {
                    // button released after debounce time
                    flimState = prevFlimState;
                    setStatusLed(flimState == fsFLiM);
                } else {
                    flimState = fsFLiMSetup;
                }
            }
            break;
        case fsTestMode:   // Button was pressed whilst in test mode
            if (!FLiM_SW) {
                flimState = fsPressedTest;
                switchTime.Val = tickGet();
            }
            break;
        case fsPressedTest:   // Button was pressed whilst in test mode
            if (FLiM_SW) {
                // button now released
                if ((tickTimeSince(switchTime) > FLiM_HOLD_TIME)) {
                    // button released after full FLiM switch time 
                    flimState = prevFlimState;
                    setStatusLed(flimState == fsFLiM);
                } else {
                    if ((tickTimeSince(switchTime) > NEXT_TEST_TIME)) {
                        // button released after time for next test selection
                        flimState = fsNextTest;
                    } else {
                        if ((tickTimeSince(switchTime) > FLiM_DEBOUNCE_TIME)) {
                            // button released after debounce time
                            flimState = fsTestInput;    // An input to the current test mode (eg: repeat test)
                        } else {
                            flimState = fsTestMode;     // Button not held down long enough to do anything, so just carry on
                        }
                    }
                }
            }
            break;
    } // switch

} // FLiMSWCheck



/**
 * Enter FLiM setup mode - called when module wants to be allocated a node number for FLiM
 * (for example, when pusbbutton pressed.)
 * Sends the node number allocated.
 */
void requestNodeNumber( void ) {
    BYTE local_cbusMsg[d0+3];

    // send request node number packet
    cbusSendOpcNN(ALL_CBUS, OPC_RQNN, -1, local_cbusMsg);  
} // requestNodeNumber


/**
 ** Revert to SLiM mode - called when module wants to go back to slim.
 * For example when pushbutton pressed long enough whilst in FLiM mode.
 */
void SLiMRevert(void) {   
    // send nn release packet
    cbusSendOpcNN(ALL_CBUS, OPC_NNREL, -1, cbusMsg);
} // SLiMrevert

/** 
 * Process CBUS opcodes. Anything module explicit will already have been dealt 
 * with in the application code before calling here.
 * 
 * @param msg the CBUS message
 * @return true if the message was processed
 */
BOOL parseCBUSMsg(BYTE *msg) {
    // Process the incoming message
    if (!parseCbusEvent(msg))
        return( parseFLiMCmd(msg));
    return FALSE;
}


/**
 * Process CBUS opcode for FLiM. Called after any module specific CBUS opcodes have
 * been dealt with.  
 * 
 * @param rx_ptr the received CBUS message
 * @return true if the message was processed
 */
BOOL parseFLiMCmd(BYTE *rx_ptr) {
#ifdef __XC8__
    BOOL     cmdProcessed = FALSE;
#else
    overlay BOOL     cmdProcessed = FALSE;
#endif


    if (flimState == fsFLiMLearn) {
        cmdProcessed = TRUE;

        switch(rx_ptr[d0]) {
        case OPC_NNLRN:     // If in learn mode and receive a learn message for another node, must exit learn mode
            if (thisNN(rx_ptr)) {
                break;  // Ignore if for us (already in learn) but drop through to exit learn mode if not addressed to us
            }
            /* fall through */
        case OPC_NNULN:
            // Release node from learn mode
             flimState = fsFLiM;
            break;
        case OPC_NNCLR:
            // Clear all events
            doNnclr;
            break;
        case OPC_EVULN:
            // Unlearn event
            doEvuln( (rx_ptr[d1] << 8) + rx_ptr[d2], (rx_ptr[d3] << 8) + rx_ptr[d4]);
            break;
        case OPC_EVLRN:
            // Teach event whilst in learn mode
            doEvlrn( (rx_ptr[d1] << 8) + rx_ptr[d2] , (rx_ptr[d3] << 8) + rx_ptr[d4], rx_ptr[d5], rx_ptr[d6]);
            break;
        case OPC_EVLRNI:
            // Teach event whilst in learn mode with event index
            doEvlrn((rx_ptr[d1] << 8) + rx_ptr[d2] , (rx_ptr[d3] << 8) + rx_ptr[d4], rx_ptr[d6], rx_ptr[d7]);  // Current implementation does not use index to save learnt event
            break;
        case OPC_REQEV:
            // Read event variable by event id
            doReqev((rx_ptr[d1] << 8) + rx_ptr[d2] , (rx_ptr[d3] << 8) + rx_ptr[d4], rx_ptr[d5]);
            break;
        default:
            cmdProcessed = FALSE;
            break;
        }
    } // in learn mode

    if (!cmdProcessed && thisNN(rx_ptr)) {
       // process commands specifically addressed to us
        cmdProcessed = TRUE;

    	switch(rx_ptr[d0]) {
#ifdef BOOTLOADER_PRESENT
            case OPC_BOOT:
                // TODO implement call to bootloader
                break;
#endif
            case OPC_RQNPN:
                // Read one node parameter by index
                doRqnpn(rx_ptr[d3]);
                break;
            case OPC_NNLRN:
                // Put node into learn mode
                if (flimState == fsFLiM)
                    flimState = fsFLiMLearn;
                break;
              case OPC_NNEVN:
                // Read available event slots
                doNnevn();
                break;
            case OPC_NERD:
                // Read all stored events
                doNerd();
                break;
            case OPC_RQEVN:
                // Read number of stored events
                doRqevn();
                break;
            case OPC_NVRD:
                // Read value of a node variable
                doNvrd(rx_ptr[d3]);
                break;
            case OPC_NVSET:
                // Set a node variable
                doNvset(rx_ptr[d3], rx_ptr[d4]);
                break;
            case OPC_REVAL:
                // Read event variable by index
                doReval();
                break;
            default:
                cmdProcessed = FALSE;
                break;
        }
    } // this NN

    if (!cmdProcessed) {   
        // Process any command not sent specifically to us that still needs action
        if (rx_ptr[d0] == OPC_QNN) {
            QNNrespond();          // Respond to node query 	//  ??? update to do new spec response agreed
            cmdProcessed = TRUE;
        }
    }

    // In setup mode, also check for FLiM commands not addressed to
    // any particular node

    if	((!cmdProcessed) && (flimState == fsFLiMSetup)) {
        cmdProcessed = TRUE;

        switch(rx_ptr[d0]) {
            case OPC_RQNP:
                    // Read node parameters
                    doRqnp();
                    break;
            case OPC_RQMN:
                    // Read module type name
                    doRqmn();
                    break;
            case OPC_SNN:
                    // Set node number
                    doSnn(rx_ptr);
                    break;
            default:
                    cmdProcessed = FALSE;
            break;
        }
    }

    return( cmdProcessed );
} // parse_FLiM_cmd


// Internal functions


/**
 *  QNN Respond.
 * Send response bytes to QNN query
 */
void QNNrespond() {
    FLiMprmptr  paramptr;

    paramptr = (FLiMprmptr)&FLiMparams;

    cbusMsg[d0] = OPC_PNN;
    cbusMsg[d3] = paramptr->params.manufacturer;
    cbusMsg[d4] = paramptr->params.module_id;
    cbusMsg[d5] = paramptr->params.module_flags;
    cbusSendMsgNN(ALL_CBUS, -1, cbusMsg);
}

/**
 * Send node number acknowledge.
 */
void doNNack( void ) {
	cbusSendOpcMyNN(0, OPC_NNACK, cbusMsg);
} // doNNack

/**
 * Read one node parameter by index.
 */
void doRqnpn(BYTE idx) {
    FLiMprmptr  paramptr;

    paramptr = (FLiMprmptr)&FLiMparams;

    if (idx <= FCUparams.parameter_count) {
        cbusMsg[d0] = OPC_PARAN;
        cbusMsg[d3] = idx;

        if (idx == 0) {
            cbusMsg[d4] = FCUparams.parameter_count;
        } else if ((idx >= PAR_CPUMID) && (idx < PAR_CPUMAN)  ) {
            cbusMsg[d4] = readCPUType() >> (( idx - PAR_CPUMID )*8);
        } else {
            cbusMsg[d4] = paramptr->bytes[idx-1];
        }
        if ((idx == PAR_FLAGS) && (flimState == fsFLiM)) {
            cbusMsg[d4] |= PF_FLiM;
        }
    	cbusSendMsgNN(ALL_CBUS, -1, cbusMsg);
    } else {
        doError(CMDERR_INV_PARAM_IDX);
    }
} // doRqnpn


/**
 * Read a node variable.
 * @param NVindex the index of the Node Variable
 */
void doNvrd(BYTE NVindex)
{
    if (NVindex > NV_NUM) {
        doError(CMDERR_INV_NV_IDX);
    } else {
        WORD flashIndex;
        flashIndex = AT_NV;
        flashIndex += NVindex;
        flashIndex--;
        
        // Get NV index and send response with value of NV
        cbusMsg[d0] = OPC_NVANS;
        cbusMsg[d3] = NVindex;
        cbusMsg[d4] = readFlashBlock(flashIndex);
        cbusSendMsgMyNN( 0, cbusMsg );
    }
} // doNvrd


/**
 * Set a node variable.
 * @param NVindex the index of the NV to be written
 * @param NVvalue the new NV value
 */
void doNvset(BYTE NVindex, BYTE NVvalue)
{
    if (NVindex > NV_NUM) {
        doError(CMDERR_INV_NV_IDX);
    } else {
        WORD flashIndex;

        // *NVPtr[--NVindex] = NVvalue; // Set value of node variable (NV counts from 1 in opcode, adjust index to count from zero)

        flashIndex = AT_NV;
        flashIndex += NVindex;
        flashIndex--;

        BYTE oldValue = *NVPtr[--NVindex];
        if (validateNV(NVindex, oldValue, NVvalue)) {
            writeFlashByte((BYTE *)flashIndex, NVvalue);
            actUponNVchange(NVindex, NVvalue);
            cbusSendOpcMyNN( 0, OPC_WRACK, cbusMsg);
        } else {
            doError(CMDERR_INV_NV_VALUE);
        }
    }
} // doNvset



/**
 * Read node parameters in setup mode.
 * Returns the first set of parameters that will fit in a CAN message.
 * Additional parameters added since this opcode was defined can only be read using RQNPN.
 */
void doRqnp(void) {
    FLiMprmptr  paramptr;
    BYTE        copyCounter;

    paramptr = (FLiMprmptr)&FLiMparams;

    cbusMsg[d0] = OPC_PARAMS;

    for (copyCounter = d1; copyCounter <= d7; copyCounter++) {
        cbusMsg[copyCounter] = paramptr->bytes[copyCounter-d1];
    }
    /* Can't send PARAM 8
    if (flimState == fsFLiM) {
        cbusMsg[d1+PAR_FLAGS-1] |= PF_FLiM;
     }
     */
    cbusSendMsg(0, cbusMsg);
} // doRqnp

/** Read node parameters in setup mode.
 * Returns the first set of parameters that will fit in a CAN message.
 * Additional parameters added since this opcode was defined can only be read using RQNPN.
 */
void doRqmn(void) {
    BYTE        copyCounter, padCounter;
#ifdef __XC8__
    char*   namptr;
#else
    rom char*   namptr;
#endif
    DWORD       namadr;
 
    namadr = FCUparams.module_type_name;
#ifdef __XC8__
    namptr = (char*)namadr;
#else
    namptr = (rom char*)namadr;
#endif

    cbusMsg[d0] = OPC_NAME;
    for (copyCounter = 0; *namptr != 0; copyCounter++ ) {
      cbusMsg[copyCounter+d1] = *namptr++;
    }
    for (padCounter = copyCounter; padCounter < 7; padCounter++ ) {
      cbusMsg[padCounter+d1] = ' ';
    }
    cbusSendMsg( 0, cbusMsg );
} // doRqmn


/**
 *  Set node number while in setup mode.
 * @param rx_ptr the CBUS message
 */
void doSnn( BYTE *rx_ptr ) {
    // Get new node number for FLiM
    nodeID = rx_ptr[d1];
    nodeID <<= 8;
    nodeID +=  rx_ptr[d2];
    flimState = fsFLiM;
    
    // Store new node id and mode to nonvol EEPROM

    SaveNodeDetails(nodeID, fsFLiM);
 
    // Acknowledge new node id

    cbusSendOpcMyNN( 0, OPC_NNACK, cbusMsg );
    setFLiMLed();
} // doSnn


/**
 * Send a CBUS error message.
 * @param code the error code - see cbusdefs8m.h
 */
void doError(unsigned int code) {
		cbusMsg[d0] = OPC_CMDERR;
		cbusMsg[d3] = code;
		cbusSendMsgNN(ALL_CBUS, -1, cbusMsg);
}

/**
 * Test to see if the CBUS message matches our NN.
 * @param rx_ptr the CBUS message
 * @return Trus if NN matches ours
 */
BOOL thisNN( BYTE *rx_ptr) {
	if (((rx_ptr[d0] >> 5) >= 2) && (((WORD)(rx_ptr[d1])<<8) + rx_ptr[d2]) == nodeID) {
	 	return TRUE;
	} else {
	 	return FALSE;
    }
}

/**
 * Save the NN and current state in EEPROM.
 * @param nodeID the NN
 * @param flimState the current state
 */
void SaveNodeDetails(WORD nodeID, enum FLiMStates flimState)
{
    ee_write_short((WORD)EE_NODE_ID, nodeID);
    ee_write((WORD)EE_FLIM_MODE, flimState);
} // SaveNodeDetails

/**
 * Blink green LED on to show busy doing commands, but overidden by flash if in FLiM setup mode.
 * @param blinkstatus
 * @return the current LED state
 */
BYTE BlinkLED( BOOL blinkstatus )
{
	BYTE LEDstatus;

	if (FLiMFlash) {
		LEDstatus = FlashStatus;
		// FlashStatus = !FlashStatus;
	} else {
		LEDstatus = blinkstatus;
    }

	return( LEDstatus ? 1 : 0 );
	
} // BlinkLED

