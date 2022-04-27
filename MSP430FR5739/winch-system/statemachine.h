/*
 * statemachine.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_


//-- StateMachine states
#define IDLE            0
#define DECODE          1

#define START_PAWL      2
#define WAIT_PAWL       3
#define START_MOTOR     4
#define WAIT_MOTOR      5
#define ENGAGE_PAWL_1   6
#define ENGAGE_PAWL_2   7

#define ABORT           8
#define SEND_TO_UCCM    9


//-- Receive commands from the uccm. top level state machine
void handle_commands(void);

//-- Decodes message sent from UCCM and outputs next state
int decode_msg(char msg[2]);

#endif /* STATEMACHINE_H_ */
