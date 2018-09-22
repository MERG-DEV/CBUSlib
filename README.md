# CBUSlib

MERG CBUS for model railway control - PIC18 CBUS libraries.

These files provide a number of helpful function that can be used to develop MERG CBUS modules. They help you to comply with
the CBUS specification.

The files are not strictly a library but a set of source files which are added to the application code to build the firmware for a module.

## Branches

- Master is the current realsed branch. It currently supports only the C18 compiler
- XC8 branch is for changes to support the XC8 compiler. Need to be merged back into master at some point.

## Usage ##
To use this library:
1. clone this repo into your local git directory.
2. Create a new project within MPLAB-X selecting:
    - Standalone Microchip project
    - Advanced 8-bit MCUs, PIC18F25K80
    - PICkit3
    - Specify which toolchain you are using
    - Specify your project name
3. Optionally clone the CANnone project and copy/rename the files to the project directory. This will provide an example main loop and the necessary defines required by the library. Edit these files as necessary.
4. Selectively "Import Existing Item" from the CBUSlib local repo.

## Release Notes ##

- Very much work in progress!
- Currently The BlinkLED does not flash the LED when processing a CBUS message.
- Lots of work required to merge XC8 branch back into master so that master can support either compiler.
- Changes to events.c to implement changes discussed between Ian Hogg / Pete Brownlow.
- Needs proper testing.
- Need documentation to explain the functionality of each file and their requirements when adding to a project.

