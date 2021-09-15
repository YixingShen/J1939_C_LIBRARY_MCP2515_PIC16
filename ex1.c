/*
Example 1
This example shows a very simple J1939 implementation. It uses polling
to check for a message to light an LED and to send a message if a
button is pressed.
j1939cfg.h should be configured as follows:
J1939_ACCEPT_CMDADD should be commented out
J1939_POLL_MCP should be uncommented
*/
#include <pic.h>
#include ".\j1939\j1939cfg.h"

J1939_USER_MSG_BANK J1939_MESSAGE Msg;

void main( void )
{
    unsigned char LastSwitch = 1;
    unsigned char CurrentSwitch;
    TRISD0 = 1; // Switch pin
    TRISD1 = 0; // LED pin
    RD1 = 0; // Turn off LED
    J1939_Initialization();

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

        // Since we don’t accept the Commanded Address message,
        // the value passed here doesn’t matter.
        J1939_Poll(1);
    }
}
/*********************************************************************/
/*********************************************************************/