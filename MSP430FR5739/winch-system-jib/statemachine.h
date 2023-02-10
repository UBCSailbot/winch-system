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

//-- HEADER MSGS 4 bits
#define SETPOS_MSG          0x1
#define QUERYPOS_MSG        0x2
#define STOPLOCK_MSG        0x3
#define ALIVE_MSG           0x4
#define BUSY_MSG            0x5
#define UNDEF_MSG           0x6

// TX MSG Structure
/* ************************************************
 * * 1 bit err * 3 bits Header *   12 bits Data   *
 * ************************************************
 */
#define HEADER_OFFSET       12
#define HEADER_MASK         0x7000
#define DATA_OFFSET         0
#define DATA_MASK           0x0FFF

#define ERROR_OFFSET        14
#define ERROR_MASK          0x8000


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
