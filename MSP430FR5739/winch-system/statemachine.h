/*
 * statemachine.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include "uart.h"

//-- StateMachine states
#define IDLE                0
#define DECODE              1

#define START_PAWL          2
#define WAIT_PAWL           3
#define START_MOTOR         4
#define WAIT_MOTOR          5
#define START_ENGAGE_PAWL   6
#define WAIT_ENGAGE_PAWL    7

#define ABORT               8
#define SEND_TO_UCCM        9

//-- UCCM MSGS
#define SETPOS_MSG          0x01
#define QUERYPOS_MSG        0x02
#define STOPLOCK_MSG        0x03
#define ALIVE_MSG           0x04


//-- Receive commands from the uccm. top level state machine
void handle_commands(void);

//-- Controls statemachine transitions
static void statemachine(char msg[RXBUF_LEN]);

//-- Decodes message sent from UCCM and outputs next state
static int decode_msg(char msg[2]);

//-- Abort functionality. Hault main motor, Engages pawl
static int abort_action(void);

#endif /* STATEMACHINE_H_ */
