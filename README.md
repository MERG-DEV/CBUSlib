# CBUSlib

MERG CBUS for model railway control - PIC18 CBUS libraries.

These files provide a number of helpful functions that can be used to 
develop MERG CBUS modules. 
They help you to comply with the CBUS specification.

The files are not strictly a library but a set of source files which 
are added to the application code to build the firmware for a module.

# Version 2m

See CANMIO wiki page for full details of changes and bug fixes in this version


## Usage ##
To use this library:
1. clone this repo into your local git directory.
2. Create a new project within MPLAB-X selecting:
    - Standalone Microchip project
    - Advanced 8-bit MCUs, PIC18F25K80 or PIC18F26K80
    - PICkit3
    - Specify which toolchain you are using
    - Specify your project name
4. Selectively "Import Existing Item" from the CBUSlib local repo.

For examples of usage see:
  * CANMIOfirmware (Universal) https://github.com/MERG-DEV/CANMIOfirmware
  * CANCOMPUTE https://github.com/MERG-DEV/CANCOMPUTE

## To Dos ##

- XC8 support.
- Improvements to documentation to explain the functionality of each file and their requirements when adding to a project.

