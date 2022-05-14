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


//-- Include uart.h in header file

unsigned int state = IDLE;
t_cmd * cur_cmd;

void handle_commands(void) {
    char msg[RXBUF_LEN] = "";
    unsigned int rx_flag;

    while (1) {
        rx_flag = isReady();
        //-- If rx_ready we have an incoming msg
        if (rx_flag) {

            //-- If the maximum active command threshold is not reached
            if (!max_active_reached()) {
                getMsg(msg);

                //-- NEED TO DO THIS SO WE CAN RETURN - If no command active then this does not do anything
                save_current_state(state);

                state = DECODE;
            }

            //-- We ignore the rx_ready flag if max active threshold has been reached
            clearReady();
        }

        statemachine(msg);

        //-- 100 Hz
        __delay_cycles(10000);
    }
}

static void statemachine(char msg[RXBUF_LEN]) {
    int ret_val = 0;

    switch(state) {
    case IDLE:
        //V_PRINTF("Idle\r\n");
        //--  Idle wait
        break;
    case DECODE:
        //V_PRINTF("decode\r\n");
        // Decode message and find next state
        state = decode_msg(msg);
        break;

    //-- SET_POS states
    case START_PAWL:
        //V_PRINTF("START_PAWL\r\n");
        //-- Send the direction data and initialize the PAWL
        ret_val = move_pawl(cur_cmd->data2, INIT_PAWL);

        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: program an error lookup
            state = ABORT;
        } else {
            state = WAIT_PAWL;
        }
        break;

    case WAIT_PAWL:
        //V_PRINTF("WAIT_PAWL\r\n");
        //-- Run pawl until action is complete. Action complete - 1, Run again - 0, Error < 0
        ret_val = move_pawl(cur_cmd->data2, RUN_PAWL);

        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else if (ret_val == 1){
            state = START_MOTOR;
        }
        break;

    case START_MOTOR:
        //V_PRINTF("START_MOTOR\r\n");
        //-- Initialize motor functions
        ret_val = setMainMotorPosition(cur_cmd->data1, &cur_cmd->data2, INIT_MMOTOR);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else{
            state = WAIT_MOTOR;
        }
        break;

    case WAIT_MOTOR:
        //V_PRINTF("WAIT_MOTOR\r\n");
        //-- Run motor until action is complete. Action complete - 1, Run again - 0, Error < 0
        ret_val = setMainMotorPosition(cur_cmd->data1, &cur_cmd->data2, RUN_MMOTOR);
        //V_PRINTF("Ret value: %d\r\n", ret_val);
        //V_PRINTF("MOTOR state: %d\r\n", P1OUT & 1);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else if (ret_val == 2) {
            //-- Change sate to START_PAWLs
            state = START_PAWL;
        } else if (ret_val == 1) {
            state = START_ENGAGE_PAWL;
        }
        break;

    case START_ENGAGE_PAWL:
        //V_PRINTF("START_ENGAGE_PAW\r\n");
        //-- Initialize Pawls to engage
        ret_val = move_pawl(REST, INIT_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else {
            state = WAIT_ENGAGE_PAWL;
        }
        break;

    case WAIT_ENGAGE_PAWL:
        //V_PRINTF("WAIT_ENGAGE_PAWL\r\n");
        //-- Run pawl control until both engaged. Action complete - 1, Run again - 0, Error < 0
        ret_val = move_pawl(REST, RUN_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else if (ret_val == 1){
            state = SEND_TO_UCCM;
        }
        break;

    case ABORT:

        //-- Primary action to stop winch functionality
        if (abort_action() < 0) {
            //-- Perform a PUC reset?
        }

        state = SEND_TO_UCCM;
        break;

    case SEND_TO_UCCM:

        //-- Send message to the UCCM
        uccm_send("%x\r\n\0", cur_cmd->msg);

        //-- Removes current command from the list
        end_command();

        //-- If we have another command resume otherwise go to IDLE
        if (is_command_available()) {
            cur_cmd = get_current_command();

            //-- Update current state to the interrupted command
            state = cur_cmd->cont_state;
        } else {
            state = IDLE;
        }
        break;
    }
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
    switch((int) (msg[0])) {

    case 0x02:            // SET_POS

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
        }

        //-- Sets data1 - pos data2 - dir and uccm_msg - pos. If busy then the command is automatically set to busy
        cur_cmd = new_command(SET_POS, pos, dir, pos);
        break;

    case 0x04:        // QUERY_POS

        err = getCurrentPosition(&pos);

        //-- data1: 0, data2: 0, uccm_msg: pot. If busy then command is set to busy type
        cur_cmd = new_command(QUERY_POS, 0, 0,pos);

        if (err < 0) {
            //-- TODO: Error check
        }

        next_state = SEND_TO_UCCM;
        break;

    case 0x06:        // STOPLOCK

        //-- We need to clear all current commands that are running so we do not return to them after
        clear_all_commands();

        cur_cmd = new_command(STOPLOCK, 0, 0,0x6);

        next_state = ABORT;
        break;

    case 0x08:        // ALIVE

        cur_cmd = new_command(ALIVE, 0, 0,  0x5555);

        next_state = SEND_TO_UCCM;
        break;

    default:            // UNDEF

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

