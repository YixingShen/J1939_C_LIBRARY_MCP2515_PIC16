#ifndef __j1939pro_h
#define __j1939pro_h

/*
j939pro.h

This header file contains the prototypes for the J1939 library routines.
These cannot be in j1939_16.h because the pointers to the user
message buffers must specify whether they are in banks 0/1 or
banks 2/3 with the J1939_USER_MSG_BANK definition, which is defined in
j1939cfg.h.

The global variable declarations are also included here, mostly to reduce
the risk that someone will accidentally change them.

This header file is automatically included by including j1939cfg.h.

Version     Date        Description
----------------------------------------------------------------------
v1.00       2003/12/11  Initial release

Copyright 2003 Kimberly Otten Software Consulting
*/


// Give visibility to the global variables.

extern J1939_FLAG    						J1939_Flags;
extern J1939_RX_QUEUE_BANK unsigned char	RXQueueCount;


// Library function prototypes

void 			J1939_AddressClaimHandling( unsigned char Mode );
#ifdef J1939_ACCEPT_CMDADD
void			J1939_CommandedAddressHandling( void );
#endif
unsigned char	J1939_DequeueMessage( J1939_USER_MSG_BANK J1939_MESSAGE *MsgPtr );
unsigned char  	J1939_EnqueueMessage( J1939_USER_MSG_BANK J1939_MESSAGE *MsgPtr );
void 			J1939_Initialization( void );
void			J1939_ISR( void );
void 			J1939_Poll( unsigned char ElapsedTime );
void 			J1939_ReceiveMessages( void );
void 			J1939_RequestForAddressClaimHandling( void );
unsigned char 	J1939_TransmitMessages( void );

#endif
