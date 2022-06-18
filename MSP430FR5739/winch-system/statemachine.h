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
enum States {
    IDLE,
    DECODE,
    TURN_MOTOR_ON,
    START_PAWL,
    WAIT_PAWL,
    START_MOTOR,
    WAIT_MOTOR,
    START_ENGAGE_PAWL,
    WAIT_ENGAGE_PAWL,
    TURN_MOTOR_OFF,
    ABORT,
    SEND_TO_UCCM,
};

#define INTERRUPT_MASK       1


//-- Receive commands from the uccm. top level state machine
void handle_commands(void);

//-- Controls statemachine transitions
enum States get_next_state(void);

//-- Decodes message sent from UCCM and outputs next state
enum States decode_msg(char msg[2]);

//-- Abort functionality. Hault main motor, Engages pawl
static int abort_action(void);

#endif /* STATEMACHINE_H_ */
