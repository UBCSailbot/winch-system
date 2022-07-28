/*
 * statemachine.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */
#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_
#include "uart.h"


typedef enum States {
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

#endif /* STATEMACHINE_H_ */
