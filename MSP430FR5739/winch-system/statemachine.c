/*
 * statemachine.c
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */
#include <msp430.h>
#include "statemachine.h"
#include "commands.h"
#include "spi.h"
#include "motor.h"
#include "pawl.h"
#include "debug.h"
//-- #include "uart.h" in statemachine.h

unsigned int state;
unsigned int interrupts;
t_cmd * cur_cmd;

void handle_commands(void) {
    char msg[RXBUF_LEN] = "";
    interrupts = INTERRUPT_MASK;
    state = IDLE;

    while (1) {

        //-- If no new messages continue to whatever state we should be in
        state = get_next_state();

        //-- Check if we have an incoming message from the UCCM
        if (isReady() && interrupts) {

            //-- We only need to process this if we havn't reached our maximum capacity
            if (!max_active_reached()) {

                getMsg(msg);

                //-- NEED TO DO THIS SO WE CAN RETURN, INCASE THIS COMMAND IS BEING INTERRUPTED - If no active command then this does not do anything
                save_current_state(state);

                //-- Decode this message to figure out the next state and creates a new command
                state = decode_msg(msg);
            }

            //-- We ignore the rx_ready flag if max active threshold has been reached
            clearReady();
        }

        //-- 100 Hz TODO: USE TIMER MODULE
        __delay_cycles(10000);
    }
}

static unsigned int get_next_state(void) {
    int ret_val = 0;
    unsigned int next_state;

    switch(state) {

    case IDLE:
        /*  DO NOTHING */
        break;

    case DECODE:
        /* State should never be decode as it is done right after receiving a UCCM msg */
        break;

    //-- SET_POS --//
    case START_PAWL:

        //-- Initialize Pawl data2 -> direction
        ret_val = move_pawl(cur_cmd->data2, INIT_PAWL);
        if (ret_val < 0) {

            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: program an error lookup
            next_state = ABORT;
        } else {

            next_state = WAIT_PAWL;
        }
        break;

    case WAIT_PAWL:

        //-- Run pawl until action is complete data2 -> direction. Action complete - 1, Run again - 0, Error - < 0
        ret_val = move_pawl(cur_cmd->data2, RUN_PAWL);
        if (ret_val < 0) {

            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            next_state = ABORT;
        } else if (ret_val == 1){

            next_state = START_MOTOR;
        }
        break;

    case START_MOTOR:

        //-- Initialize motor functions data1 -> Position, data2 -> Direction
        ret_val = setMainMotorPosition(cur_cmd->data1, &cur_cmd->data2, INIT_MMOTOR);
        if (ret_val < 0) {

            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            next_state = ABORT;
        } else{

            next_state = WAIT_MOTOR;
        }
        break;

    case WAIT_MOTOR:

        //-- Run motor until action is complete data1 -> Position, data2 -> Direction. Action complete - 1, Run again - 0, Error < 0
        ret_val = setMainMotorPosition(cur_cmd->data1, &cur_cmd->data2, RUN_MMOTOR);
        if (ret_val < 0) {

            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            next_state = ABORT;
        } else if (ret_val == 2) {

            //-- Change sate to START_PAWLs
            next_state = START_PAWL;
        } else if (ret_val == 1) {

            next_state = START_ENGAGE_PAWL;
        }
        break;

    case START_ENGAGE_PAWL:

        //-- Initialize Pawls to engage
        ret_val = move_pawl(REST, INIT_PAWL);
        if (ret_val < 0) {

            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            next_state = ABORT;
        } else {

            next_state = WAIT_ENGAGE_PAWL;
        }
        break;

    case WAIT_ENGAGE_PAWL:

        //-- Run pawl control until both engaged. Action complete - 1, Run again - 0, Error < 0
        ret_val = move_pawl(REST, RUN_PAWL);
        if (ret_val < 0) {

            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            next_state = ABORT;
        } else if (ret_val == 1){

            next_state = SEND_TO_UCCM;
        }
        break;

    case ABORT:

        //-- Primary action to stop winch functionality
        if (abort_action() < 0) {
            //-- Perform a PUC reset?
        }

        next_state = SEND_TO_UCCM;
        break;

    case SEND_TO_UCCM:

        //-- Send message to the UCCM
        uccm_send("%c%c\r\n", cur_cmd->msg >> 8, cur_cmd->msg & 0xFF);

        //-- Removes current command from the list
        end_command();

        //-- Reset interrupt for instances when it is disabled
        interrupts |= INTERRUPT_MASK;

        //-- If we have another command resume otherwise go to IDLE
        if (is_command_available()) {

            cur_cmd = get_current_command();

            //-- Update next state to the interrupted command
            next_state = cur_cmd->cont_state;
        } else {

            next_state = IDLE;
        }
        break;
    }

    return next_state;
}

/*
 *  Decodes the message from the UCCM and creates a new command
 *  Returns the next state depending on the new command (next state logic)
 *
 *  Types of commands:
 *      - SET_POS
 *      - QUERY_POS
 *      - STOPLOCK
 *      - ALIVE
 *      - UNDEF
 *      - ACTION_BUSY
 */
static int decode_msg(char msg[2]) {
    unsigned int pos, dir;
    int err;
    unsigned int next_state = IDLE;

    // The first bytes xxxx_xxx0 ignoring the least significant bit is the identifier
    switch((int) (msg[0]) >> 1) {

    case 0x01:            // SET_POS

        //-- If this action is busy we dont calculate pos and dir but create a busy command instead
        if (!is_busy(SET_POS)) {

            //-- Byte[0] 0000_000x << 8 + Byte[1] xxxx_xxxx
            pos = ((int)msg[0] & 0x1) << 8 | msg[1];

            err = getDirection(pos, &dir);

            if (err < 0) {
                //-- TODO: Do something?
            }

            next_state = START_PAWL;
        } else {
            next_state = SEND_TO_UCCM;
            interrupts &= ~INTERRUPT_MASK;
        }

        //-- Sets data1 - pos data2 - dir and uccm_msg - pos. If busy then the command is automatically set to busy
        cur_cmd = new_command(SET_POS, pos, dir, pos);
        break;

    case 0x02:        // QUERY_POS

        err = getCurrentPosition(&pos);

        //-- We want to disable interrupts because we don't want busy commands to be interrupted
        if (is_busy(QUERY_POS)) interrupts &= ~INTERRUPT_MASK;

        //-- data1: 0, data2: 0, uccm_msg: pot. If busy then command is set to busy type
        cur_cmd = new_command(QUERY_POS, 0, 0,pos);

        if (err < 0) {
            //-- TODO: Error check
        }

        next_state = SEND_TO_UCCM;
        break;

    case 0x03:        // STOPLOCK

        //-- We need to clear all current commands that are running so we do not return to them after
        clear_all_commands();

        //-- Interrupts are always disabled for stop and lock
        interrupts &= ~INTERRUPT_MASK;

        cur_cmd = new_command(STOPLOCK, 0, 0,0x6);

        next_state = ABORT;
        break;

    case 0x04:        // ALIVE

        //-- We want to disable interrupts because we don't want busy commands to be interrupted
        if (is_busy(ALIVE)) interrupts &= ~INTERRUPT_MASK;

        cur_cmd = new_command(ALIVE, 0, 0,  0x5555);

        next_state = SEND_TO_UCCM;
        break;

    default:            // UNDEF

        //-- We want to disable interrupts because we don't want busy commands to be interrupted
        if (is_busy(UNDEF)) interrupts &= ~INTERRUPT_MASK;

        cur_cmd = new_command(UNDEF, 0, 0,  0xFF00);

        next_state = SEND_TO_UCCM;
        break;
    }
    return next_state;
}

/*
 * Haults the main motor and Engages Pawl
 */
static int abort_action(void) {
   int err;

    //-- Hault motor
   stopMainMotor();

    //-- Engage pawl
   err = move_pawl(REST, INIT_PAWL);
   if (err < 0) {
       return -1;
   }

   //-- Perform this function until either error or returns 1
   do {
       err = move_pawl(REST, RUN_PAWL);

       //-- If there something is wrong error
       if (err < 0) {
           return -2;
       }

       //-- 100 Hz
       __delay_cycles(10000);
   } while (err != 1);

   //-- Turn off power to the main motor
   turnOffMotor();

   return 0;
}



