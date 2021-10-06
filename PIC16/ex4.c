/*
Example 3b
This example shows what the receiving node for Example 3a should look
like, using the same concept as Example 2 of using interrupts to check
for a message to light an LED and to send a message if a button is
pressed. Note that three basic changes are required:
- it must accept the Commanded Address message (j1939cfg.h)
- it must have a CA_AcceptCommandedAddress function
- it must call J1939_Poll during the main loop, even though interrupts are being used.
The rest of the code is identical. The change of address will be
handled in the background.
j1939cfg.h should be configured as follows:
J1939_ACCEPT_CMDADD should be uncommented
J1939_POLL_MCP should be commented out
*/
#include <pic.h>
#include ".\j1939\j1939cfg.h"

J1939_USER_MSG_BANK J1939_MESSAGE Msg;

unsigned char CA_AcceptCommandedAddress( void )
{
    return 1;
}

#pragma interrupt_level 0
void interrupt isr( void )
{
    if (INTF)
        J1939_ISR();
}

void main( void )
{
    unsigned char LastSwitch = 1;
    unsigned char CurrentSwitch;
    TRISD0 = 1; // Switch pin
    TRISD1 = 0; // LED pin
    RD1 = 0; // Turn off LED
    
    J1939_Initialization();
    
    GIE = 1;

    // Wait for address contention to time out
    while (J1939_Flags.Flags.WaitingForAddressClaimContention)
    {
        DelayMilliseconds(1);
        J1939_Poll(1);
    }

    // Now we know our address should be good, so start checking for
    // messages and switches.
    while (1)
    {
        CurrentSwitch = RD0;
        
        if (LastSwitch != CurrentSwitch)
        {
            Msg.Msg.DataPage = 0;
            Msg.Msg.Priority = 7;
            Msg.Msg.DestinationAddress = OTHER_NODE;
            Msg.Msg.DataLength = 0;
            
            if (CurrentSwitch == 0)
                Msg.Msg.PDUFormat = TURN_ON_LED;
            else
                Msg.Msg.PDUFormat = TURN_OFF_LED;
        
            while (J1939_EnqueueMessage( &Msg ) != RC_SUCCESS);
            
            LastSwitch = CurrentSwitch;
            
        }
        
        while (RXQueueCount > 0)
        {
            J1939_DequeueMessage( &Msg );
        
            if (Msg.Msg.PDUFormat == TURN_ON_LED)
                RD1 = 1;
            else if (Msg.Msg.PDUFormat == TURN_OFF_LED)
                RD1 = 0;
        }
        
        // We need to call J1939_Poll since we can accept the
        // Commanded Address message. Now the time value passed in
        // is important.
        J1939_Poll( MAIN_LOOP_TIME_IN_MS );
    } //while (1)
}
/*********************************************************************/
/*********************************************************************/