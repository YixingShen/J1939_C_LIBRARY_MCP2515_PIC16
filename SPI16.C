/*
spi16.c

SPI Interface Functions for PIC16 Devices

These functions were written to mimic the SPI library functions
as found in the MPLAB C18 library.  The functionality is the same
as the PIC18 library, with modifications for the PIC16 implementation
and the HI-TECH PICC compiler.

This file has been optimized for use with the J1939 C Library.
Unused functions have been commented out, and a provision has been
included for making the ReadSPI and WriteSPI functions in-line.  Also,
an interrupt level definition is included for the WriteSPI function.

This file requires the following header files:
	pic.h (HI-TECH)
	j1939cfg.h
	j1939pro.h
	j1939_16.h
	mcp2515.h
	spi16.h

Version     Date        Description
----------------------------------------------------------------------
v1.00       2003/12/11  Initial release

Copyright 2003 Kimberly Otten Software Consulting
*/

#include <pic.h>
#include "spi16.h"
#include "j1939cfg.h"

/********************************************************************
Function Name:    CloseSPI
Description:      This function disables the SSP module. Pin
                  I/O returns under the control of the TRISC
                  and LATC registers.
Parameters:       None
Return Value:     None
********************************************************************/
#pragma inline CloseSPI
void CloseSPI( void )
{
    SSPEN = 0;
}


/********************************************************************
Function Name:    DataRdySPI
Description:      This function determines if there is a byte in the
                  SSPBUF register.
Parameters:       None
Return Value:     0 if the serial port buffer is empty
                  1 if the serial port buffer is full
********************************************************************/
/* This function isn't needed for the J1939 library
#pragma inline DataRdySPI
unsigned char DataRdySPI( void )
{
    if ( STAT_BF )
        return ( 1 );
    else
        return ( 0 );
}
*/

/********************************************************************
Function Name:    getsSPI
Description:      This routine reads a string from the SPI
                  bus.  The number of bytes to read is deter-
                  mined by parameter 'length'.
Parameters:       rdptr     Address of read string storage location
                  length    Length of string bytes to read
Return Value:     None
********************************************************************/
/* This function isn't needed for the J1939 library
void getsSPI( unsigned char *rdptr, unsigned char length )
{
    while ( length )
    {
        *rdptr++ = getcSPI();
        length--;
    }
}
*/

/********************************************************************
Function Name:  OpenSPI
Description:    This function sets up the SSP module on a
                   PIC16 device for use with an SPI device.
Parameters:     sync_mode   SPP mode; must be one of the following
                            values from SPI16.H:
                                SPI_FOSC_4, SPI_FOSC_16, SPI_FOSC_64,
                                SPI_FOSC_TMR2, SLV_SSON, SLV_SSOFF
                bus_mode    SPI bus mode; must be one of the following
                            values from SPI16.H:
                                MODE_00, MODE_01, MODE_10, MODE_11
                smp_phase   SPI data input sample phase; must be one
                            of the following values from SPI16.H:
                                SMPEND, SMPMID
Return Value:   None
********************************************************************/
#pragma inline OpenSPI
void OpenSPI( unsigned char sync_mode, unsigned char bus_mode, unsigned char smp_phase)
{
    SSPSTAT  = 0x00;                //   Reset SSPSTAT and SSPCON to
    SSPCON   = 0x00;                // their POR states.

    SSPCON   |= sync_mode;          //   Select the serial mode and
    STAT_SMP |= smp_phase;          // the data input sample phase.

    switch( bus_mode )
    {
        case MODE_00:
            STAT_CKE = 1;           //   Data is transmitted on rising edge.
            break;
        case MODE_10:
            STAT_CKE = 1;           //   Data is transmitted on falling edge.
            CKP      = 1;           //   Clock idle state high.
            break;
        case MODE_11:
            CKP      = 1;           //   Clock idle state high.
            break;
        default:                    //   Default SPI bus mode 0,1
            break;
    }

    switch( sync_mode )
    {
        case SLV_SSON:
            SPI_SS_PIN_TRIS = 1;    //   Define /SS pin as an input. Then fall through.
        case SLV_SSOFF:
            SPI_CLOCK_PIN_TRIS = 1; //   Define clock pin as an input.  In
            STAT_SMP = 0;           // slave mode, SMP must be cleared.
            break;
        default:                    //   Default master mode.
            SPI_CLOCK_PIN_TRIS = 0; //   Define clock pin as an output.
            break;
    }

    SPI_SDO_PIN_TRIS = 0;           //   Define SDO as an output (master or slave).
    SPI_SDI_PIN_TRIS = 1;           //   Define SDI as an input (master or slave)
    SSPEN = 1;                      //   Enable the synchronous serial port.
}


/********************************************************************
Function Name:    putsSPI
Description:      This routine writes a null terminated string to the
                  SPI bus.
Parameters:       wrptr     Address of the null terminated string
Return Value:     None
********************************************************************/
/* This function isn't needed for the J1939 library
void putsSPI( unsigned char *wrptr )
{
    while ( *wrptr )
    {
        SSPBUF = *wrptr++;          //   Write one byte to SSPBUF, then
        while( !STAT_BF );          // wait until BF bit is set.
    }
}
*/

/********************************************************************
Function Name:    ReadSPI
Description:      Read one byte from the SPI bus.
Parameters:       None
Return Value:     Contents of SSPBUF register
********************************************************************/
#ifndef SPI_USE_ONLY_INLINE_DEFINITIONS
unsigned char ReadSPI( void )
{
    SSPBUF = 0x00;                  //   Initiate a bus cycle by writing
    while ( !STAT_BF );             // to SSPBUF.  Wait until a received
    return ( SSPBUF );              // byte is ready, and return it.
}
#endif

/********************************************************************
Function Name:    WriteSPI
Description:      This routine writes a single byte to the SPI bus.
Parameters:       data_out      Single data byte for SPI bus
Return Value:     0             Byte transmitted successfully
                  -1            Write collision
********************************************************************/
#ifndef SPI_USE_ONLY_INLINE_DEFINITIONS
#ifndef J1939_POLL_MCP
#pragma interrupt_level 0
#endif
unsigned char WriteSPI( unsigned char data_out )
{
    SSPBUF = data_out;              //   Write the data byte to SSPBUF
    if ( WCOL )                     // register.  If a write collision
        return ( -1 );              // occurred, return an error condition.
    else                            // Otherwise, wait until the SSPBUF
    {                               // is empty and return success.
        while( !STAT_BF );
    }
    return ( 0 );
}
#endif

