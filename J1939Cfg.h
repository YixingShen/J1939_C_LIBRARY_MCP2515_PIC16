#ifndef __j1939cfg_h
#define __j1939cfg_h

/*
j1939cfg.h

This file will configure the J1939 library for a specific application.
Refer to the J1939 C Library User Guide for more information on how
to use the library and the purpose of each item below.

This file requires the following header files:
	j1939pro.h
	j1939_16.h
	mcp2515.h
	spi16.h

Version     Date        Description
----------------------------------------------------------------------
v1.00       2003/12/11  Initial release

Copyright 2003 Kimberly Otten Software Consulting
*/

#include "spi16.h"
#include "mcp2515.h"
#include "j1939_16.h"

#define J1939_TRUE	1
#define J1939_FALSE	0

// Define the address configuration and capability of the CA.  If the
// Commanded Address message can be accepted, uncomment the following
// line.  Otherwise, comment it out to save program memory.

//#define J1939_ACCEPT_CMDADD


// Define the CA's initial address on the J1939 network.  Note that this
// can be changed during run time if the CA accepts the Commanded Address
// message.

#define J1939_STARTING_ADDRESS	128


// Define the CA's initial NAME.  Note that this can be changed during run
// time by the CA, but the system must be reinitialized on the network.
// For example, suppose a CA's NAME fields have the following values:
// (refer to the SAE J1939 Specification for these values)
//		Arbitrary Address Capable (1 bit)	0
//		Industry Group (3 bits)				3		 (Construction Equipment)
//		Vehicle System Instance	(4 bits)	0		 (first instance)
//		Vehicle System (7 bits)				0		 (Non-specific System)
//		(Reserved) (1 bit)					0
//		Function (8 bits)					0x81	 (Laser Receiver)
//		Function Instance (5 bits)			0		 (first instance)
//		ECU Instance (3 bits)				0		 (first instance)
//		Manufacturer Code (11 bits)			8	 	 (Caterpillar, Inc.)
//		Identity Number	(21 bits)			0x16D35A (unique for the application)
// The complete NAME value would then be: 0x300081000116D35A
// This value should be mapped into the following #defines:
//		#define J1939_CA_NAME7	0x30
//		#define J1939_CA_NAME6	0x00
//		#define J1939_CA_NAME5	0x81
//		#define J1939_CA_NAME4	0x00
//		#define J1939_CA_NAME3	0x01
//		#define J1939_CA_NAME2	0x16
//		#define J1939_CA_NAME1	0xD3
//		#define J1939_CA_NAME0	0x5A

#define J1939_CA_NAME7	0x30
#define J1939_CA_NAME6	0x00
#define J1939_CA_NAME5	0x81
#define J1939_CA_NAME4	0x00
#define J1939_CA_NAME3	0x01
#define J1939_CA_NAME2	0x16
#define J1939_CA_NAME1	0xD3
#define J1939_CA_NAME0	0x5A


// J1939 Message Setup

// Define the bank where the CA will keep any messages it is processing,
// either for transmission or upon reception.  Note that all messages
// must be defined within the same bank.  This bank does not have to
// match the bank of either of the queues.

#define J1939_USER_MSG_BANK			bank1

// Define the receive queue size, bank, and whether or not the last
// location of the queue will be overwritten if a message is received
// when the queue is full.

#define J1939_RX_QUEUE_SIZE			1
#define J1939_RX_QUEUE_BANK			bank2
#define J1939_OVERWRITE_RX_QUEUE	J1939_FALSE

// Define the transmit queue size, bank, and whether or not the last
// location of the queue will be overwritten if a message is enqueued
// when the queue is full.

#define J1939_TX_QUEUE_SIZE			5
#define J1939_TX_QUEUE_BANK			bank3
#define J1939_OVERWRITE_TX_QUEUE	J1939_FALSE


// Stack vs. ROM Configuration

// If the following line is uncommented, one less stack level will be used,
// but more program memory words will be required.  This is recommended only
// if more stack space is absolutely required, since it is very expensive in ROM.

//#define SPI_USE_ONLY_INLINE_DEFINITIONS


// If the CA uses the MCP2515's INT pin on the PIC's INT pin, comment
// out the following definition.  Otherwise, uncomment the definition.

//#define J1939_POLL_MCP


// Define which PIC pin is connected to the MCP2515 CS pin.  Note that both the
// pin and the TRIS must be defined, and these must match.

#define J1939_CS_PIN	RC0
#define J1939_CS_TRIS	TRISC0


// Define the SPI configuration: mode, speed, sample phase, clock out usage,
// and clock out prescaler.  Refer to mcp2515.h for valid values.

#define J1939_SPI_MODE	MODE_00
#define J1939_SPI_SPEED	SPI_FOSC_4
#define J1939_SAMPLE	SMPEND
#define J1939_CLKOUT	CLKOUT_DISABLE
#define J1939_CLKOUT_PS	CLKOUT_PS1


// Bit Rate Calculations.
//
// Refer to the MCP2515 data sheet for complete information on setting
// up these three values.  Refer to mcp2515.h for helpful constants.

#define J1939_CNF1		(SJW1 + (8-1))
#define J1939_CNF2		(BTLMODE + (3-1)*8 + (1-1))
#define J1939_CNF3		(SOF_DISABLE + WAKFIL_DISABLE + (3-1))


// Include the function prototypes header file.  This file requires
// definitions that are declared above.  It also gives access to
// global variables.

#include "j1939pro.h"

#endif
