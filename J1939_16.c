/*
j1939_6.c

This file contains the library routines for the J1939 C Library for PIC16
devices.  Please refer to the J1939 C Library User Guide for information
on configuring and using this library.

This file requires the following files to be linked:
	spi16.c

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
v1.01		2004/01/28	Corrected Request/Response mechanism

Copyright 2004 Kimberly Otten Software Consulting
*/

 #include	<pic.h>

#include "j1939cfg.h"		// Also includes spi16.h, mcp2515.h, J1939_16.h, and j1939pro.h


// Internal definitions

#define ADDRESS_CLAIM_TX	1
#define ADDRESS_CLAIM_RX	2


// Global variables.  Some of these will be visible to the CA.

J1939_RX_QUEUE_BANK unsigned char		CA_Name[J1939_DATA_LENGTH];
unsigned char 							CommandedAddress;
unsigned char							CommandedAddressSource;
#ifdef J1939_ACCEPT_CMDADD
	J1939_RX_QUEUE_BANK unsigned char 	CommandedAddressName[J1939_DATA_LENGTH];
#endif
unsigned char 							ContentionWaitTime;
unsigned char 							J1939_Address;
J1939_FLAG    							J1939_Flags;
J1939_TX_QUEUE_BANK J1939_MESSAGE 		OneMessage;

J1939_RX_QUEUE_BANK unsigned char RXHead;
J1939_RX_QUEUE_BANK unsigned char RXTail;
J1939_RX_QUEUE_BANK unsigned char RXQueueCount;
J1939_RX_QUEUE_BANK J1939_MESSAGE RXQueue[J1939_RX_QUEUE_SIZE];

J1939_TX_QUEUE_BANK unsigned char TXHead;
J1939_TX_QUEUE_BANK unsigned char TXTail;
J1939_TX_QUEUE_BANK unsigned char TXQueueCount;
J1939_TX_QUEUE_BANK J1939_MESSAGE TXQueue[J1939_TX_QUEUE_SIZE];


// Code definitions for common functions, to make it a little easier to read.

#define SELECT_MCP		J1939_CS_PIN = 0;
#define UNSELECT_MCP 	J1939_CS_PIN = 1;


// Function Prototypes

#ifdef J1939_ACCEPT_CMDADD
	unsigned char CA_AcceptCommandedAddress( void );
#endif

/*********************************************************************
CompareName

This routine compares the passed in array data NAME with the CA's
current NAME as stored in CA_Name.

Parameters:	unsigned char *		Array of NAME bytes
Return:		-1 - CA_Name is less than the data
		 	0  - CA_Name is equal to the data
			1  - CA_Name is greater than the data
*********************************************************************/
#ifndef J1939_POLL_MCP
#pragma interrupt_level 0
#endif
signed char CompareName( J1939_RX_QUEUE_BANK unsigned char *OtherName )
{
	unsigned char	i;

	for (i = 0; (i<J1939_DATA_LENGTH) && (OtherName[i] == CA_Name[i]); i++);

	if (i == J1939_DATA_LENGTH)
		return 0;
	else if (CA_Name[i] < OtherName[i] )
		return -1;
	else
		return 1;
}

/*********************************************************************
CopyName

This routine copies the CA's NAME into the message buffer's data array.
We can afford to make this a function, since another function is always
called after this is done, and we won't be using any additional stack
space.

Parameters:	None
Return:		None
*********************************************************************/
void CopyName(void)
{
	unsigned char i;

	for (i=0; i<J1939_DATA_LENGTH; i++)
		OneMessage.Msg.Data[i] = CA_Name[i];
}

/*********************************************************************
SetAddressFilter

This routine sets filter 2 to the specified value (destination address).
It is used to allow reception of messages sent to this node specifically
or simply to the global address if this node does not have an address
(Address will be J1939_GLOBAL_ADDRESS).

NOTE: We can only use one stack level from here, so MCP_Write calls
have been replaced by their equivalent inline code.

Parameters:	unsigned char	J1939 Address of this CA
Return:		None
*********************************************************************/
#ifndef J1939_POLL_MCP
#pragma interrupt_level 0
#endif
void SetAddressFilter( unsigned char Address )
{
	unsigned char	Status;

	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_WRITE );
		WRITESPI( MCP_CANCTRL );
		WRITESPI( MODE_CONFIG + J1939_CLKOUT + J1939_CLKOUT_PS );
	#else
		WriteSPI( MCP_WRITE );
		WriteSPI( MCP_CANCTRL );
		WriteSPI( MODE_CONFIG + J1939_CLKOUT + J1939_CLKOUT_PS );
	#endif
	UNSELECT_MCP;
//	MCP_Write( MCP_CANCTRL, MODE_CONFIG + J1939_CLKOUT + J1939_CLKOUT_PS );

	do
	{
		SELECT_MCP;
		#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
			WRITESPI( MCP_READ );
			WRITESPI( MCP_CANSTAT );
			READSPI( Status );
		#else
			WriteSPI( MCP_READ );
			WriteSPI( MCP_CANSTAT );
			Status = ReadSPI();
		#endif
		UNSELECT_MCP;
	} while ((Status & MODE_MASK) != MODE_CONFIG);

	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_WRITE );
		WRITESPI( MCP_RXF2EID8 );
		WRITESPI( Address );
	#else
		WriteSPI( MCP_WRITE );
		WriteSPI( MCP_RXF2EID8 );
		WriteSPI( Address );
	#endif
	UNSELECT_MCP;

	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_WRITE );
		WRITESPI( MCP_CANCTRL );
		WRITESPI( MODE_NORMAL + J1939_CLKOUT + J1939_CLKOUT_PS );
	#else
		WriteSPI( MCP_WRITE );
		WriteSPI( MCP_CANCTRL );
		WriteSPI( MODE_NORMAL + J1939_CLKOUT + J1939_CLKOUT_PS );
	#endif
	UNSELECT_MCP;
//	MCP_Write( MCP_CANCTRL, MODE_NORMAL + J1939_CLKOUT + J1939_CLKOUT_PS );
}

/*********************************************************************
MCP_Modify

This function modifies the designated MCP address.

Parameters:	unsigned char Address	Address of the MCP register.
			unsigned char Mask		Bits to modify in the register
			unsigned char Data		Data value for the MCP register.
Return:		None
*********************************************************************/
/*

This function has been brought in-line to save stack space.

void MCP_Modify( unsigned char Address, unsigned char Mask, unsigned char Data )
{
	SELECT_MCP;
	WriteSPI( MCP_BITMOD );
	WriteSPI( Address );
	WriteSPI( Mask );
	WriteSPI( Data );
	UNSELECT_MCP;
}
*/

/*********************************************************************
MCP_Write

This function writes a value to the designated MCP address.

Parameters:	unsigned char Address	Address of the MCP register.
			unsigned char Data		Data value for the MCP register.
Return:		None
*********************************************************************/
void MCP_Write( unsigned char Address, unsigned char Data )
{
	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_WRITE );
		WRITESPI( Address );
		WRITESPI( Data );
	#else
		WriteSPI( MCP_WRITE );
		WriteSPI( Address );
		WriteSPI( Data );
	#endif
	UNSELECT_MCP;
}

/*********************************************************************
SendOneMessage

This routine sends the message located at the pointer passed in.  It
also uses the message's data length field to determine how much of the
data to load.  At this point, all of the data fields, such as data
length, priority, and source address, must be set.  This routine will
set up the CAN bits, such as the extended identifier bit and the
remote transmission request bit.

NOTE: Only transmit buffers 0 and 1 are used, to guarantee that the
messages appear on the bus in the order that they are sent to the
MCP2515.

Parameters:	J1939_MESSAGE far *		Pointer to message to send
Return:		None
*********************************************************************/
#ifndef J1939_POLL_MCP
#pragma interrupt_level 0
#endif
void SendOneMessage( J1939_TX_QUEUE_BANK J1939_MESSAGE *MsgPtr )
{
//	unsigned char oldSIDL;
	unsigned char MCP_Load;
	unsigned char Loop;
	unsigned char MCP_Send;
	unsigned char Temp;

	// Set up the final pieces of the message and make sure DataLength isn't
	// out of spec.

	MsgPtr->Msg.Res = 0;
	MsgPtr->Msg.RTR = 0;
	if (MsgPtr->Msg.DataLength > 8)
		MsgPtr->Msg.DataLength = 8;

	// Put PDUFormat into the structure to match the J1939-CAN format.  This
	// involves splitting the original value in PDUFormat into two pieces,
	// leaving some holes for the TXBnSIDL register, and setting the EXIDE bit.

//	oldSIDL = MsgPtr->Msg.PDUFormat;
	MsgPtr->Msg.PDUFormat_Top = MsgPtr->Msg.PDUFormat >> 5;		// Put the top three bits into SID5-3
	Temp = MsgPtr->Msg.PDUFormat & 0x03;						// Save the bottom two bits.
	MsgPtr->Msg.PDUFormat = (MsgPtr->Msg.PDUFormat & 0x1C) << 3;// Move up bits 4-2 into SID2-0.
	MsgPtr->Msg.PDUFormat |= Temp | 0x08;						// Put back EID17-16, set EXIDE.

	// Decide which transmit buffer to use.  Lower chip select, and then
	// do a Read Status command.  Look at the transmit status bits
	// to see which buffer is ready.  We may need a time-out here.

	MCP_Load = 0;
	while (MCP_Load == 0)
	{
		SELECT_MCP;
		#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
			WRITESPI( MCP_READ_STATUS );
			READSPI( Temp );
		#else
			WriteSPI( MCP_READ_STATUS );
			Temp = ReadSPI();
		#endif
		UNSELECT_MCP;
		if (!(Temp & 0x04))
		{
			MCP_Load = MCP_LOAD_TX0;
			MCP_Send = MCP_RTS_TX0;
		}
		else if (!(Temp & 0x10))
		{
			MCP_Load = MCP_LOAD_TX1;
			MCP_Send = MCP_RTS_TX1;
		}
	}

	// Load the message buffer.  Lower the chip select line, and point
	// the loader to TXB0SIDH. Send out the first 5 bytes of the message,
	// then send out whatever part of the data is necessary.  Then raise
	// the chip select line.

	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_Load );
	#else
		WriteSPI( MCP_Load );
	#endif
	for (Loop=0; Loop<J1939_MSG_LENGTH+MsgPtr->Msg.DataLength;  Loop++)
	{
		Temp = MsgPtr->Array[Loop];
		#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
			WRITESPI( Temp );
		#else
			WriteSPI( Temp );
		#endif
	}
	UNSELECT_MCP;

	// Now tell the MCP to send the message.  Lower the chip select line,
	// and send the RTS command for the selected buffer.  Then raise the
	// chip select line.

	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_Send );
	#else
		WriteSPI( MCP_Send );
	#endif
	UNSELECT_MCP;

	#ifndef J1939_POLL_MCP
		// Clear the transmit interrupt flag
		SELECT_MCP;
		#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
			WRITESPI( MCP_BITMOD );
			WRITESPI( MCP_CANINTF );
			WRITESPI( MCP_Send<<2 );
			WRITESPI( 0 );
		#else
			WriteSPI( MCP_BITMOD );
			WriteSPI( MCP_CANINTF );
			WriteSPI( MCP_Send<<2 );
			WriteSPI( 0 );
		#endif
		UNSELECT_MCP;
	#endif

	// Put back PDUFormat the way it was in case somebody reuses the message.
//	MsgPtr->Msg.PDUFormat = oldSIDL;
}

/*********************************************************************
J1939_AddressClaimHandling

This routine is called either when the CA must claim its address on
the bus or another CA is trying to claim the same address on the bus and
we must either defend ourself or relinquish the address.  If the CA has
an address in the proprietary range of 0-127 or 248-253, it can take
the address immediately.

Parameters:	unsigned char	ADDRESS_CLAIM_RX indicates an Address
							Claim message has been received and this
							CA must either defend or give up its
							address.
							ADDRESS_CLAIM_TX indicates that the CA is
							initiating a claim to its address.
Return:		None
*********************************************************************/
#ifndef J1939_POLL_MCP
#pragma interrupt_level 0
#endif
void J1939_AddressClaimHandling( unsigned char Mode )
{
	// Get most of the message ready here, since it'll be very similar
	// for both messages to be sent.  Note that J1939_PF_ADDRESS_CLAIMED
	// is the same value as J1939_PF_CANNOT_CLAIM_ADDRESS.  We can't copy
	// the data yet because we might need to look at the old data.

	OneMessage.Msg.Priority = J1939_CONTROL_PRIORITY;
	OneMessage.Msg.PDUFormat = J1939_PF_ADDRESS_CLAIMED;
	OneMessage.Msg.DestinationAddress = J1939_GLOBAL_ADDRESS;
	OneMessage.Msg.DataLength = J1939_DATA_LENGTH;

	if (Mode == ADDRESS_CLAIM_TX)
		goto SendAddressClaim;

	if (OneMessage.Msg.SourceAddress != J1939_Address)
		return;
	if (CompareName( OneMessage.Msg.Data ) != -1) // Our CA_Name is not less
	{
		// Send Cannot Claim Address message
		CopyName();
		OneMessage.Msg.SourceAddress = J1939_NULL_ADDRESS;
		SendOneMessage( (J1939_TX_QUEUE_BANK J1939_MESSAGE *) &OneMessage );

		// Set up MCP filter 2 to receive messages sent to the global address
		SetAddressFilter( J1939_GLOBAL_ADDRESS );

		J1939_Flags.Flags.CannotClaimAddress = 1;
		J1939_Flags.Flags.WaitingForAddressClaimContention = 0;
		return;
	}

SendAddressClaim:
	// Send Address Claim message for CommandedAddress
	CopyName();
	OneMessage.Msg.SourceAddress = CommandedAddress;
	SendOneMessage( (J1939_TX_QUEUE_BANK J1939_MESSAGE *) &OneMessage );

	if (((CommandedAddress & 0x80) == 0) ||			// Addresses 0-127
		((CommandedAddress & 0xF8) == 0xF8))		// Addresses 248-253 (254,255 illegal)
	{
		J1939_Flags.Flags.CannotClaimAddress = 0;
		J1939_Address = CommandedAddress;

		// Set up MCP filter 2 to receive messages sent to this address
		SetAddressFilter( J1939_Address );
	}
	else
	{
		// We don't have a proprietary address, so we need to wait.
 		J1939_Flags.Flags.WaitingForAddressClaimContention = 1;
		ContentionWaitTime = 0;
	}
}

/*********************************************************************
J1939_DequeueMessage

This routine takes a message from the receive queue and places it in
the caller's buffer.  If there is no message to return, an appropriate
return code is returned.

Parameters:	J1939_MESSAGE *		Pointer to the caller's message buffer
Return:		RC_SUCCESS			Message dequeued successfully
			RC_QUEUEEMPTY		No messages to return
			RC_CANNOTRECEIVE	System cannot currently receive
								messages.  This will be returned only
								after the receive queue is empty.
*********************************************************************/
unsigned char J1939_DequeueMessage( J1939_USER_MSG_BANK J1939_MESSAGE *MsgPtr )
{
	unsigned char	rc = RC_SUCCESS;

	#ifndef J1939_POLL_MCP
		INTE = 0;
	#endif

	if (RXQueueCount == 0)
	{
		if (J1939_Flags.Flags.CannotClaimAddress)
			rc = RC_CANNOTRECEIVE;
		else
			rc = RC_QUEUEEMPTY;
	}
	else
	{
		*MsgPtr = RXQueue[RXHead];
		RXHead ++;
		if (RXHead >= J1939_RX_QUEUE_SIZE)
			RXHead = 0;
		RXQueueCount --;
	}

	#ifndef J1939_POLL_MCP
		INTE = 1;
	#endif

	return rc;
}

/*********************************************************************
J1939_EnqueueMessage

This routine takes a message from the caller's buffer and places it in
the transmit queue.  If the message cannot be queued or sent, an appropriate
return code is returned.  If interrupts are being used, then the
transmit interrupt is enabled after the message is queued.

Parameters:	J1939_MESSAGE *		Pointer to the caller's message buffer
Return:		RC_SUCCESS			Message dequeued successfully
			RC_QUEUEFULL		Transmit queue full; message not queued
			RC_CANNOTTRANSMIT	System cannot currently transmit
								messages.
*********************************************************************/
unsigned char J1939_EnqueueMessage( J1939_USER_MSG_BANK J1939_MESSAGE *MsgPtr )
{
	unsigned char	rc = RC_SUCCESS;

	#ifndef J1939_POLL_MCP
		INTE = 0;
	#endif

	if (J1939_Flags.Flags.CannotClaimAddress)
		rc = RC_CANNOTTRANSMIT;
	else
	{
		if ((TXQueueCount < J1939_TX_QUEUE_SIZE) ||
			 (J1939_OVERWRITE_TX_QUEUE == J1939_TRUE))
		{
			if (TXQueueCount < J1939_TX_QUEUE_SIZE)
			{
				TXQueueCount ++;
				TXTail ++;
				if (TXTail >= J1939_TX_QUEUE_SIZE)
					TXTail = 0;
			}
			TXQueue[TXTail] = *MsgPtr;

			#ifndef J1939_POLL_MCP
				// Enable the transmit interrupts on TXB0 and TXB1
				SELECT_MCP;
				#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
					WRITESPI( MCP_BITMOD );
					WRITESPI( MCP_CANINTE );
					WRITESPI( MCP_TX_INT );
					WRITESPI( MCP_TX01_INT );
				#else
					WriteSPI( MCP_BITMOD );
					WriteSPI( MCP_CANINTE );
					WriteSPI( MCP_TX_INT );
					WriteSPI( MCP_TX01_INT );
				#endif
				UNSELECT_MCP;
			#endif
		}
		else
			rc = RC_QUEUEFULL;

	}

	#ifndef J1939_POLL_MCP
		INTE = 1;
	#endif

	return rc;
}

/*********************************************************************
J1939_Initialization

This routine is called on system initialization.  It initializes
global variables, microcontroller peripherals and interrupts, and the
MCP2515.  It then starts the process of claiming the CA's address.

NOTE: This routine will NOT enable global interrupts.  The CA needs
to do that when it's ready.

Parameters:		None
Return:			None
*********************************************************************/
#pragma inline J1939_Initialization
void J1939_Initialization( void )
{
	unsigned char	i;
//	unsigned char	j;

	// Initialize global variables;
	J1939_Flags.FlagVal = 1;	// Cannot Claim Address, all other flags cleared.
	ContentionWaitTime = 0;
	CommandedAddress = J1939_Address = J1939_STARTING_ADDRESS;
	TXHead = 0;
	TXTail = 0xFF;
	TXQueueCount = 0;
	RXHead = 0;
	RXTail = 0xFF;
	RXQueueCount = 0;
	CA_Name[7] = J1939_CA_NAME7;
	CA_Name[6] = J1939_CA_NAME6;
	CA_Name[5] = J1939_CA_NAME5;
	CA_Name[4] = J1939_CA_NAME4;
	CA_Name[3] = J1939_CA_NAME3;
	CA_Name[2] = J1939_CA_NAME2;
	CA_Name[1] = J1939_CA_NAME1;
	CA_Name[0] = J1939_CA_NAME0;

	// Initialize the SPI peripheral.
	CloseSPI();
	OpenSPI( J1939_SPI_SPEED, J1939_SPI_MODE, J1939_SAMPLE );

	// Initialize the chip select pin
	J1939_CS_TRIS = 0;
	J1939_CS_PIN = 1;

	// Initialize the MCP2515 and put it into configuration mode automatically.
	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_RESET );
	#else
		WriteSPI( MCP_RESET );
	#endif
	UNSELECT_MCP;

	// Wait for the MCP to start up
	for (i=0; i<128; i++ );

	// Set up the MCP receive buffer 0 mask to receive broadcast messages.
	// Set up receive buffer 1 mask to receive messages sent to the
	// global address (or eventually us).  Set up bit timing as defined by
	// the CA, and set up interrupts on either receive buffer full.
	MCP_Write( MCP_RXM0SIDH, 0x07 );		// RXM0SIDH
	MCP_Write( MCP_RXM0SIDL, 0x80 );		// RXM0SIDL
	MCP_Write( MCP_RXM1EID8, 0xFF );		// RXM1EID8
	MCP_Write( MCP_CNF3, J1939_CNF3 );		// CNF3
	MCP_Write( MCP_CNF2, J1939_CNF2 );		// CNF2
	MCP_Write( MCP_CNF1, J1939_CNF1 );		// CNF1

	// Set all the RXB0 filters to accept only broadcast messages
	// (PF = 240-255).  Set all the RXB1 filters to accept only the
	// global address.  Once we get an address for the CA, we'll change
	// filter 2 to accept that address.
	MCP_Write( MCP_RXF0SIDH, 0x07 );					// RXF0SIDH
	MCP_Write( MCP_RXF0SIDL, 0x88 );					// RXF0SIDL
	MCP_Write( MCP_RXF1SIDH, 0x07 );					// RXF1SIDH
	MCP_Write( MCP_RXF1SIDL, 0x88 );					// RXF1SIDL
	MCP_Write( MCP_RXF2SIDL, 0x08 );					// RXF2SIDL
	MCP_Write( MCP_RXF2EID8, J1939_GLOBAL_ADDRESS );	// RXF2EID8
	MCP_Write( MCP_RXF3SIDL, 0x08 );					// RXF3SIDL
	MCP_Write( MCP_RXF3EID8, J1939_GLOBAL_ADDRESS );	// RXF3EID8
	MCP_Write( MCP_RXF4SIDL, 0x08 );					// RXF4SIDL
	MCP_Write( MCP_RXF4EID8, J1939_GLOBAL_ADDRESS );	// RXF4EID8
	MCP_Write( MCP_RXF5SIDL, 0x08 );					// RXF5SIDL
	MCP_Write( MCP_RXF5EID8, J1939_GLOBAL_ADDRESS );	// RXF5EID8

	// Put the MCP2515 into Normal Mode
	MCP_Write( MCP_CANCTRL, MODE_NORMAL + J1939_CLKOUT + J1939_CLKOUT_PS );

	// Clear out and deactivate the transmit and receive buffers.
	/*
    for (i = 0; i < 3; i++)
	{
		SELECT_MCP;
		#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
			WRITESPI( MCP_WRITE );
			WRITESPI( MCP_TXB0CTRL + i*16 );
    		for (j = 0; j < 14; j++)
        	    WRITESPI( 0x00 );
		#else
			WriteSPI( MCP_WRITE );
			WriteSPI( MCP_TXB0CTRL + i*16 );
    		for (j = 0; j < 14; j++)
        	    WriteSPI( 0x00 );
		#endif
		UNSELECT_MCP;
    }
	*/

	// Enable the MCP interrupt.  The caller must enable global
	// interrupts when ready.
	#ifndef J1939_POLL_MCP
		TRISB0 = 1;		// Set RB0 as an input.
		INTEDG = 0;  	// Interrupt on falling edge of RB0/INT pin
		INTE = 1;		// Enable the RB0/INT interrupt.
		MCP_Write( MCP_CANINTE, MCP_RX_INT );
	#endif

	// Start the process of claiming our address
	J1939_AddressClaimHandling( ADDRESS_CLAIM_TX );
}

/*********************************************************************
J1939_ISR

This function is called by the CA if it gets an interrupt and the INTF
flag is set.  First we'll clear the interrupt flag.  Then we'll call
the receive and transmit functions to process any received messages and
to transmit any messages in the transmit queue.

Parameters:	None
Return:		None
*********************************************************************/
#ifndef J1939_POLL_MCP
#pragma interrupt_level 0
#pragma inline J1939_ISR
void J1939_ISR( void )
{
	INTF = 0;

	J1939_ReceiveMessages();
	J1939_TransmitMessages();
}
#endif

/*********************************************************************
J1939_Poll

If we're polling for messages, then this routine should be called by
the CA every few milliseconds during CA processing.  The routine receives
any messages that are waiting and transmits any messages that are queued.
Then we see if we are waiting for an address contention response.  If
we are and we've timed out, we accept the address as ours.

If the CA is using interrupts, then this routine should be called by
the CA every few milliseconds while the WaitingForAddressClaimContention
flag is set after calling J1939_Initialization.  If the Commanded Address
message can be accepted, this routine must also be called every few
milliseconds during CA processing in case we are commanded to change our
address.  If using interrupts, this routine will not check for received
or transmit messages; it will only check for a timeout on address
claim contention.

Parameters:	unsigned char	The number of milliseconds that have
							passed since the last time this routine was
							called.  This number can be approximate,
							and to meet spec, should be rounded down.
Return:		None
*********************************************************************/
void J1939_Poll( unsigned char ElapsedTime )
{
	unsigned int	Temp;

	// Update the Contention Wait Time.  We have to do that before
	// we call J1939_ReceiveMessages in case the time gets reset back
	// to zero in that routine.

	Temp = ContentionWaitTime + ElapsedTime;
	if (Temp > 255)
		Temp = 255;
	ContentionWaitTime = (unsigned char) Temp;

	#ifdef J1939_POLL_MCP
		J1939_ReceiveMessages();
		J1939_TransmitMessages();
	#endif

	if (J1939_Flags.Flags.WaitingForAddressClaimContention &&
		(ContentionWaitTime >= 250))
	{
		J1939_Flags.Flags.CannotClaimAddress = 0;
		J1939_Flags.Flags.WaitingForAddressClaimContention = 0;
		J1939_Address = CommandedAddress;

		// Set up MCP filter 2 to receive messages sent to this address.
		// If we're using interrupts, make sure that interrupts are disabled
		// around this section, since it will mess up what we're doing.
		#ifndef J1939_POLL_MCP
			INTE = 0;
		#endif
		SetAddressFilter( J1939_Address );
		#ifndef J1939_POLL_MCP
			INTE = 1;
		#endif
	}
}

/*********************************************************************
J1939_ReceiveMessage

This routine is called either when an interrupt is received from the
MCP2515 or by polling.  If a message has been received, it is read in.
If it is a network management message, it is processed.  Otherwise, it
is placed in the receive queue for the user.  Note that interrupts are
disabled during this routine, since it is called from the interrupt handler.


NOTE: To save stack space, the function J1939_CommandedAddressHandling
was brought inline.

Parameters:	None
Return:		None
*********************************************************************/

void J1939_ReceiveMessages( void )
{
	unsigned char	Status;
	unsigned char	Mask = MCP_RX0IF;
	unsigned char	Loop;

	SELECT_MCP;
	#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
		WRITESPI( MCP_READ_STATUS );
		READSPI( Status );
	#else
		WriteSPI( MCP_READ_STATUS );
		Status = ReadSPI();
	#endif
	UNSELECT_MCP;

	Status &= (MCP_RX0IF | MCP_RX1IF);
	while (Status != 0)
	{
		if (Status & Mask)
		{
			// Read a message from the receive buffer
			SELECT_MCP;
			#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
				if (Mask == MCP_RX0IF)
					WRITESPI( MCP_READ_RX0 )
				else
					WRITESPI( MCP_READ_RX1 )
				for (Loop=0; Loop<J1939_MSG_LENGTH; Loop++)
					READSPI( OneMessage.Array[Loop] );
				if (OneMessage.Msg.DataLength > 8)
					OneMessage.Msg.DataLength = 8;
				for (Loop=0; Loop<OneMessage.Msg.DataLength; Loop++)
					READSPI( OneMessage.Msg.Data[Loop] );
			#else
				if (Mask == MCP_RX0IF)
					WriteSPI( MCP_READ_RX0 );
				else
					WriteSPI( MCP_READ_RX1 );
				for (Loop=0; Loop<J1939_MSG_LENGTH; Loop++)
					OneMessage.Array[Loop] = ReadSPI();
				if (OneMessage.Msg.DataLength > 8)
					OneMessage.Msg.DataLength = 8;
				for (Loop=0; Loop<OneMessage.Msg.DataLength; Loop++)
					OneMessage.Msg.Data[Loop] = ReadSPI();
			#endif
			UNSELECT_MCP;

			// Clear the receive flag, regardless of using polling or interrupts
			SELECT_MCP;
			#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
				WRITESPI( MCP_BITMOD );
				WRITESPI( MCP_CANINTF );
				WRITESPI( Mask );
				WRITESPI( 0 );
			#else
				WriteSPI( MCP_BITMOD );
				WriteSPI( MCP_CANINTF );
				WriteSPI( Mask );
				WriteSPI( 0 );
			#endif
			UNSELECT_MCP;

			// Format the PDU Format portion so it's easier to work with.
			Loop = (OneMessage.Msg.PDUFormat & 0xE0) >> 3;			// Get SID2-0 ready.
			OneMessage.Msg.PDUFormat = (OneMessage.Msg.PDUFormat & 0x03) |
										Loop |
										((OneMessage.Msg.PDUFormat_Top & 0x07) << 5);

			switch( OneMessage.Msg.PDUFormat )
			{
#ifdef J1939_ACCEPT_CMDADD
				case J1939_PF_CM_BAM:
					if ((OneMessage.Msg.Data[0] == J1939_BAM_CONTROL_BYTE) &&
						(OneMessage.Msg.Data[5] == J1939_PGN0_COMMANDED_ADDRESS) &&
						(OneMessage.Msg.Data[6] == J1939_PGN1_COMMANDED_ADDRESS) &&
						(OneMessage.Msg.Data[7] == J1939_PGN2_COMMANDED_ADDRESS))
					{
						J1939_Flags.Flags.GettingCommandedAddress = 1;
						CommandedAddressSource = OneMessage.Msg.SourceAddress;
					}
					break;
				case J1939_PF_DT:
					if ((J1939_Flags.Flags.GettingCommandedAddress == 1) &&
						(CommandedAddressSource == OneMessage.Msg.SourceAddress))
					{	// Commanded Address Handling
						if ((!J1939_Flags.Flags.GotFirstDataPacket) &&
							(OneMessage.Msg.Data[0] == 1))
						{
							for (Loop=0; Loop<7; Loop++)
								CommandedAddressName[Loop] = OneMessage.Msg.Data[Loop+1];
							J1939_Flags.Flags.GotFirstDataPacket = 1;
						}
						else if ((J1939_Flags.Flags.GotFirstDataPacket) &&
							(OneMessage.Msg.Data[0] == 2))
						{
							CommandedAddressName[7] = OneMessage.Msg.Data[1];
							CommandedAddress = OneMessage.Msg.Data[2];
							if ((CompareName( CommandedAddressName ) == 0) &&	// Make sure the message is for us.
								CA_AcceptCommandedAddress())					// and we can change the address.
								J1939_AddressClaimHandling( ADDRESS_CLAIM_TX );
							J1939_Flags.Flags.GotFirstDataPacket = 0;
							J1939_Flags.Flags.GettingCommandedAddress = 0;
						}
						else	// This really shouldn't happen, but just so we don't drop the data packet
							goto PutInReceiveQueue;
					}
					else
						goto PutInReceiveQueue;
					break;
#endif
			case J1939_PF_REQUEST:
				if ((OneMessage.Msg.Data[0] == J1939_PGN0_REQ_ADDRESS_CLAIM) &&
					(OneMessage.Msg.Data[1] == J1939_PGN1_REQ_ADDRESS_CLAIM) &&
					(OneMessage.Msg.Data[2] == J1939_PGN2_REQ_ADDRESS_CLAIM))
					J1939_RequestForAddressClaimHandling();
				else
					goto PutInReceiveQueue;
				break;
				case J1939_PF_ADDRESS_CLAIMED:
					J1939_AddressClaimHandling( ADDRESS_CLAIM_RX );
					break;
				default:
PutInReceiveQueue:
					if ((RXQueueCount < J1939_RX_QUEUE_SIZE) ||
						(J1939_OVERWRITE_RX_QUEUE == J1939_TRUE))
					{
						if (RXQueueCount < J1939_RX_QUEUE_SIZE)
						{
							RXQueueCount ++;
							RXTail ++;
							if (RXTail >= J1939_RX_QUEUE_SIZE)
								RXTail = 0;
						}
						RXQueue[RXTail] = OneMessage;
					}
					else
						J1939_Flags.Flags.ReceivedMessagesDropped = 1;
			}
		}
		Status &= ~Mask;
		Mask = MCP_RX1IF;
	}
}

/*********************************************************************
J1939_RequestForAddressClaimHandling

This routine is called if we're received a Request for Address Claim
message.  If we cannot claim an address, we send out a Cannot Claim
Address message.  Otherwise, we send out an Address Claim message for
our address.

NOTE:	J1939_PF_CANNOT_CLAIM_ADDRESS is the same value as
		J1939_PF_ADDRESS_CLAIMED, so we can reduce code size by
		combining the two.  Only the source address changes between
		the two messages.
*********************************************************************/
void J1939_RequestForAddressClaimHandling( void )
{
	if (J1939_Flags.Flags.CannotClaimAddress)
		OneMessage.Msg.SourceAddress = J1939_NULL_ADDRESS;	// Send Cannot Claim Address message
	else
		OneMessage.Msg.SourceAddress = J1939_Address;		// Send Address Claim for current address

	OneMessage.Msg.Priority = J1939_CONTROL_PRIORITY;
	OneMessage.Msg.PDUFormat = J1939_PF_ADDRESS_CLAIMED;	// Same as J1939_PF_CANNOT_CLAIM_ADDRESS
	OneMessage.Msg.DestinationAddress = J1939_GLOBAL_ADDRESS;
	OneMessage.Msg.DataLength = J1939_DATA_LENGTH;
	CopyName();
	SendOneMessage( (J1939_TX_QUEUE_BANK J1939_MESSAGE *) &OneMessage );
}

/*********************************************************************
J1939_TransmitMessages

This routine transmits as many messages from the transmit queue as it
can.  If the system cannot transmit messages, an error code is returned.
Note that interrupts are disabled during this routine, since it is
called from the interrupt handler.

Parameters:	None
Return:		RC_SUCCESS			Message was transmitted successfully
			RC_CANNOTTRANSMIT	System cannot transmit messages.
								Either we cannot claim an address or
								the MCP2515 is busy.
			RC_QUEUEEMPTY		Transmit queue was empty
*********************************************************************/
unsigned char J1939_TransmitMessages( void )
{
	unsigned char Mask = 0x04;
	unsigned char Status;

	if (TXQueueCount != 0)
	{
		if (J1939_Flags.Flags.CannotClaimAddress)
			return RC_CANNOTTRANSMIT;

		SELECT_MCP;
		#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
			WRITESPI( MCP_READ_STATUS );
			READSPI( Status );
			Status &= MCP_TX01_MASK;				// Save only the transmit request flags.
		#else
			WriteSPI( MCP_READ_STATUS );
			Status = ReadSPI() & MCP_TX01_MASK;		// Save only the transmit request flags.
		#endif
		UNSELECT_MCP;

		if (Status == MCP_TX01_MASK)			// All transmit buffers are busy
			return RC_CANNOTTRANSMIT;

		while ((TXQueueCount > 0) && (Mask != 0))
		{
			if ((Status & Mask) == 0)	// This buffer is free
			{
				TXQueue[TXHead].Msg.SourceAddress = J1939_Address;
				SendOneMessage( (J1939_TX_QUEUE_BANK J1939_MESSAGE *) &(TXQueue[TXHead]) );
				TXHead ++;
				if (TXHead >= J1939_TX_QUEUE_SIZE)
					TXHead = 0;
				TXQueueCount --;
			}
			Mask <<= 2;
		}

		#ifndef J1939_POLL_MCP
			// Disable the transmit interrupt if the queue is empty
			if (TXQueueCount == 0)
			{
				SELECT_MCP;
				#ifdef SPI_USE_ONLY_INLINE_DEFINITIONS
					WRITESPI( MCP_BITMOD );
					WRITESPI( MCP_CANINTE );
					WRITESPI( MCP_TX_INT );
					WRITESPI( MCP_NO_INT );
				#else
					WriteSPI( MCP_BITMOD );
					WriteSPI( MCP_CANINTE );
					WriteSPI( MCP_TX_INT );
					WriteSPI( MCP_NO_INT );
				#endif
				UNSELECT_MCP;
			}
		#endif

		return RC_SUCCESS;
	}
	return RC_QUEUEEMPTY;
}


