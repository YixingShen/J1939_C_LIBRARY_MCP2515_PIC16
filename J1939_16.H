#ifndef __J1939_16_H
#define __J1939_16_H

/*
j1939_16.h

This file contains constants that are used by the J1939 C Library.

Version     Date        Description
----------------------------------------------------------------------
v1.00       2003/12/11  Initial release
v1.01		2004/01/28	Added useful #define labels

Copyright 2004 Kimberly Otten Software Consulting
*/


// Function Return Codes

#define RC_SUCCESS				0
#define RC_QUEUEEMPTY			1
#define RC_QUEUEFULL			1
#define RC_CANNOTRECEIVE		2
#define RC_CANNOTTRANSMIT		2
#define RC_PARAMERROR			3


// J1939 Default Priorities

#define J1939_CONTROL_PRIORITY			0x03
#define J1939_INFO_PRIORITY				0x06
#define J1939_PROPRIETARY_PRIORITY		0x06
#define J1939_REQUEST_PRIORITY			0x06
#define J1939_ACK_PRIORITY				0x06
#define J1939_TP_CM_PRIORITY			0x07
#define J1939_TP_DT_PRIORITY			0x07


// J1939 Defined Addresses

#define J1939_GLOBAL_ADDRESS			255
#define J1939_NULL_ADDRESS				254


// Some J1939 PDU Formats, Control Bytes, and PGN's

#define J1939_PF_REQUEST2					201
#define J1939_PF_TRANSFER					202

#define J1939_PF_ACKNOWLEDGMENT				232
#define J1939_ACK_CONTROL_BYTE				0
#define J1939_NACK_CONTROL_BYTE				1
#define J1939_ACCESS_DENIED_CONTROL_BYTE	2
#define J1939_CANNOT_RESPOND_CONTROL_BYTE	3

#define J1939_PF_REQUEST					234

#define J1939_PF_DT							235		// Data Transfer message

#define J1939_PF_TP_CM						236		// Connection Management message
#define J1939_PF_CM_BAM						236		// Connection Management message (old label)
#define J1939_RTS_CONTROL_BYTE				16		// Request to Send control byte of CM message
#define J1939_CTS_CONTROL_BYTE				17		// Clear to Send control byte of CM message
#define J1939_EOMACK_CONTROL_BYTE			19		// End of Message control byte of CM message
#define J1939_BAM_CONTROL_BYTE				32		// BAM control byte of CM message
#define J1939_CONNABORT_CONTROL_BYTE		255		// Connection Abort control byte of CM message

#define J1939_PGN2_REQ_ADDRESS_CLAIM		0x00
#define J1939_PGN1_REQ_ADDRESS_CLAIM		0xEA
#define J1939_PGN0_REQ_ADDRESS_CLAIM		0x00

#define J1939_PGN2_COMMANDED_ADDRESS		0x00
#define J1939_PGN1_COMMANDED_ADDRESS		0xFE	// (-81 PDU Format)
#define J1939_PGN0_COMMANDED_ADDRESS		0xD8	// (-81 PDU Specific)

#define J1939_PF_ADDRESS_CLAIMED			238		// With global address
#define J1939_PF_CANNOT_CLAIM_ADDRESS		238		// With null address
#define J1939_PF_PROPRIETARY_A				239
#define J1939_PF_PROPRIETARY_B				255


// J1939 Data Structures

// The J1939_MESSAGE_STRUCT is designed to map the J1939 messages pieces
// into the address map for the MCP2515.  Only the field PDU Format does
// not cleanly map into the MCP2515 registers.  Users of the structure
// should simply use the field PDUFormat and ignore PDUFormat_Top.  Adjustments
// will be made immediately upon reception and just prior to transmission.

// Note: HI-TECH creates structures from low bit position to high bit
// position, so the order may appear not to match the MCP2515 registers.

#define J1939_MSG_LENGTH	5
#define J1939_DATA_LENGTH	8

struct J1939_MESSAGE_STRUCT {
	unsigned int	PDUFormat_Top		: 3;	// This needs pre and post processing.
	unsigned int	DataPage			: 1;
	unsigned int	Res					: 1;
	unsigned int	Priority			: 3;
	unsigned char	PDUFormat;					// CA should use only PDUFormat.
	unsigned char	PDUSpecific;
	unsigned char	SourceAddress;
	unsigned int	DataLength 			: 4;
	unsigned int	RTR					: 4;	// RTR bit, value always 0x00
	unsigned char	Data[J1939_DATA_LENGTH]; };

#define GroupExtension PDUSpecific
#define DestinationAddress PDUSpecific

union J1939_MESSAGE_UNION {
	struct J1939_MESSAGE_STRUCT	Msg;
	unsigned char				Array[J1939_MSG_LENGTH + J1939_DATA_LENGTH];
	};

typedef union J1939_MESSAGE_UNION J1939_MESSAGE;


struct J1939_FLAG_STRUCT {
	unsigned int	CannotClaimAddress				: 1;
	unsigned int	WaitingForAddressClaimContention: 1;
	unsigned int	GettingCommandedAddress			: 1;
	unsigned int	GotFirstDataPacket				: 1;
	unsigned int	ReceivedMessagesDropped			: 1; };

union J1939_FLAGS_UNION {
	struct J1939_FLAG_STRUCT	Flags;
	unsigned char				FlagVal; };

typedef union J1939_FLAGS_UNION J1939_FLAG;



#endif
