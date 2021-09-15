/*
Example 3a
This example shows the same concept as Example 2, using interrupts to
check for a message to light an LED and to send a message if a button
is pressed. But for the first 5 button presses, the message is sent to
the wrong address. On the 5th push, the Commanded Address message is
sent to command the other node to use the address that this node is
sending the message to. Note that this node doesn’t even need to know
what the other node’s first address is, as long as it knows the node’s
NAME.
j1939cfg.h should be configured as follows:
J1939_ACCEPT_CMDADD should be commented out
J1939_POLL_MCP should be commented out
*/
#include <pic.h>
#include ".\j1939\j1939cfg.h"

J1939_USER_MSG_BANK J1939_MESSAGE Msg;

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
    unsigned charPushCount = 0;
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
            Msg.Msg.DestinationAddress = SECOND_ADDRESS;
            Msg.Msg.DataLength = 0;

            if (CurrentSwitch == 0)
                Msg.Msg.PDUFormat = TURN_ON_LED;
            else
            {
                Msg.Msg.PDUFormat = TURN_OFF_LED;
                if (PushCount < 6)
                PushCount ++;
            }

            while (J1939_EnqueueMessage( &Msg ) != RC_SUCCESS);

            LastSwitch = CurrentSwitch;

            if (PushCount == 5)
            {
                Msg.Msg.DataPage = 0;
                Msg.Msg.Priority = 7;
                Msg.Msg.DestinationAddress = J1939_GLOBAL_ADDRESS;
                Msg.Msg.DataLength = 8;
                Msg.Msg.PDUFormat = J1939_PF_CM_BAM;
                Msg.Msg.Data[0] = 1939_BAM_CONTROL_BYTE;
                Msg.Msg.Data[1] = 9; // 9 data bytes
                Msg.Msg.Data[2] = 0;
                Msg.Msg.Data[3] = 2; // 2 packets
                Msg.Msg.Data[4] = 0xFF; // Reserved
                Msg.Msg.Data[5] = J1939_PGN0_COMMANDED_ADDRESS;
                Msg.Msg.Data[6] = J1939_PGN1_COMMANDED_ADDRESS;
                Msg.Msg.Data[7] = J1939_PGN2_COMMANDED_ADDRESS;
                
                while (J1939_EnqueueMessage( &Msg ) != RC_SUCCESS);
                
                Msg.Msg.DataPage = 0;
                Msg.Msg.Priority = 7;
                Msg.Msg.DestinationAddress = J1939_GLOBAL_ADDRESS;
                Msg.Msg.DataLength = 8;
                Msg.Msg.PDUFormat = J1939_PF_DT;
                Msg.Msg.Data[0] = 1; // First packet
                Msg.Msg.Data[1] = NODE_NAME0;
                Msg.Msg.Data[2] = NODE_NAME1;
                Msg.Msg.Data[3] = NODE_NAME2;
                Msg.Msg.Data[4] = NODE_NAME3;
                Msg.Msg.Data[5] = NODE_NAME4;
                Msg.Msg.Data[6] = NODE_NAME5;
                Msg.Msg.Data[7] = NODE_NAME6;
                
                while (J1939_EnqueueMessage( &Msg ) != RC_SUCCESS);
                
                Msg.Msg.Data[0] = 2; // Second packet
                Msg.Msg.Data[1] = NODE_NAME7;
                Msg.Msg.Data[2] = SECOND_ADDRESS;
                Msg.Msg.Data[3] = 0xFF;
                Msg.Msg.Data[4] = 0xFF;
                Msg.Msg.Data[5] = 0xFF;
                Msg.Msg.Data[6] = 0xFF;
                Msg.Msg.Data[7] = 0xFF;
                
                while (J1939_EnqueueMessage( &Msg ) != RC_SUCCESS);
                
                while (RXQueueCount > 0)
                {
                    J1939_DequeueMessage( &Msg );
                    
                    if (Msg.Msg.PDUFormat == TURN_ON_LED)
                       RD1 = 1;
                    else if (Msg.Msg.PDUFormat == TURN_OFF_LED)
                        RD1 = 0;
                }
            } //if (PushCount == 5)
        } //if (LastSwitch != CurrentSwitch)
    } //while (1)
}
/*********************************************************************/
/*********************************************************************/