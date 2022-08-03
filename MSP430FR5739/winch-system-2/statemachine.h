/*
 * statemachine.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */
#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include "uart.h"
#include "return_codes.h"

typedef enum States {
    IDLE,
    DECODE,
    SET_DIRECTION,
    TURN_MOTOR_ON,
    START_PAWL,
    WAIT_PAWL,
    START_MOTOR,
    WAIT_MOTOR,
    START_ENGAGE_PAWL,
    WAIT_ENGAGE_PAWL,
    TURN_MOTOR_OFF,
    GET_POSITION,
    ABORT,
    SEND_TO_UCCM,
    ERROR_STATE,

    MAX_STATE
} t_state;

//-- UCCM MSGS
#define SETPOS_MSG          0x01
#define QUERYPOS_MSG        0x02
#define STOPLOCK_MSG        0x03
#define ALIVE_MSG           0x04
#define BUSY_MSG            0x08


//-- Receive commands from the uccm. top level state machine
void handle_commands(void);

//-- Controls statemachine transitions
static void statemachine(void);

//-- Decodes message sent from UCCM and outputs next state
t_state decode_msg(void);

//-- Abort functionality. Hault main motor, Engages pawl
static int abort_action(void);

//-- Uses a next state lookup to get the next state depending on the retunred code
t_state get_next_state(t_state current_state, t_ret_code returned_code);

#endif /* STATEMACHINE_H_ */
