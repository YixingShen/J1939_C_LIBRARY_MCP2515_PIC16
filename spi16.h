#ifndef __SPI16_H
#define __SPI16_H

/*
spi16.h

This file is the PIC16 SPI peripheral header file.  It contains constants
for use with the SPI port on PIC16 devices.

Version     Date        Description
----------------------------------------------------------------------
v1.00       2003/12/11  Initial release

Copyright 2003 Kimberly Otten Software Consulting
*/

// Define the SPI pins on the PIC

#define SPI_SDI_PIN_TRIS	TRISC4
#define SPI_SDO_PIN_TRIS	TRISC5
#define SPI_CLOCK_PIN_TRIS	TRISC3
#define SPI_SS_PIN_TRIS		TRISA5


// SSPSTAT Register Values

// Master SPI mode only

#define   SMPEND        0x80           // Input data sample at end of data out
#define   SMPMID        0x00           // Input data sample at middle of data out

#define   MODE_00       0              // Setting for SPI bus Mode 0,0
//CKE           0x40                   // SSPSTAT register
//CKP           0x00                   // SSPCON1 register

#define   MODE_01       1              // Setting for SPI bus Mode 0,1
//CKE           0x00                   // SSPSTAT register
//CKP           0x00                   // SSPCON1 register

#define   MODE_10       2              // Setting for SPI bus Mode 1,0
//CKE           0x40                   // SSPSTAT register
//CKP           0x10                   // SSPCON1 register

#define   MODE_11       3              // Setting for SPI bus Mode 1,1
//CKE           0x00                   // SSPSTAT register
//CKP           0x10                   // SSPCON1 register


// SSPCON Register Values

#define   SPI_FOSC_4    0              // SPI Master mode, clock = Fosc/4
#define   SPI_FOSC_16   1              // SPI Master mode, clock = Fosc/16
#define   SPI_FOSC_64   2              // SPI Master mode, clock = Fosc/64
#define   SPI_FOSC_TMR2 3              // SPI Master mode, clock = TMR2 output/2
#define   SLV_SSON      4              // SPI Slave mode, /SS pin control enabled
#define   SLV_SSOFF     5              // SPI Slave mode, /SS pin control disabled


// Function Prototypes

void			CloseSPI( void );
unsigned char 	DataRdySPI( void );
unsigned char 	ReadSPI( void );
void 			OpenSPI( unsigned char sync_mode, unsigned char bus_mode, unsigned char smp_phase );
unsigned char 	WriteSPI( unsigned char data_out );
void 			getsSPI( unsigned char *rdptr, unsigned char length );
void 			putsSPI( unsigned char *wrptr );


// Define alternate function names

#define  getcSPI  ReadSPI
#define  putcSPI  WriteSPI

// Uncomment the #define for SPI_USE_ONLY_INLINE_DEFINITIONS in j1939cfg.h
// to use these inline alternates and save stack space.  Note that a significant
// amount of ROM will be required, so these should be used only if more stack space
// is absolutely required.

#define READSPI( Val )						\
					{						\
						SSPBUF = 0x00;		\
					    while ( !STAT_BF ); \
					    Val = SSPBUF;    \
					}

// Since we're doing things inline, we have to remove the error checking.

#define WRITESPI( Val )							\
					{							\
					    SSPBUF = Val;    		\
					    if ( !WCOL )         	\
					        while( !STAT_BF );	\
					}


#endif  /* __SPI16_H */

