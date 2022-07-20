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

<<<<<<< Updated upstream
            //-- If the maximum active command threshold is not reached
            if (!max_active_reached()) {
                getMsg(msg);
=======
            getMsg(rx_msg);
            add_new_command((unsigned int) rx_msg);
>>>>>>> Stashed changes

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
<<<<<<< Updated upstream
        //V_PRINTF("Idle\r\n");
        //--  Idle wait
        break;
    case DECODE:
        //V_PRINTF("decode\r\n");
=======

        //--  Idle wait
        break;
    case DECODE:

>>>>>>> Stashed changes
        // Decode message and find next state
        state = decode_msg(msg);
        break;

    case TURN_MOTOR_ON:

        turnOnMotor();
        state = START_PAWL;
        break;

    //-- SET_POS states
    case START_PAWL:

        //-- Send the direction data and initialize the PAWL
        ret_val = move_pawl(INIT_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: program an error lookup
            state = ABORT;
        } else {
            state = WAIT_PAWL;
        }
        break;

    case WAIT_PAWL:

        //-- Run pawl until action is complete. Action complete - 1, Run again - 0, Error < 0
        ret_val = move_pawl(RUN_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else if (ret_val == 1){
            state = START_MOTOR;
        }
        break;

    case START_MOTOR:

        //-- Initialize motor functions
        ret_val = setMainMotorPosition(INIT_MMOTOR);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else{
            state = WAIT_MOTOR;
        }
        break;

    case WAIT_MOTOR:

        //-- Run motor until action is complete. Action complete - 1, Run again - 0, Error < 0
        ret_val = setMainMotorPosition(RUN_MMOTOR);
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

        //-- Initialize Pawls to engage
        ret_val = engageBoth(INIT_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else {
            state = WAIT_ENGAGE_PAWL;
        }
        break;

    case WAIT_ENGAGE_PAWL:

        //-- Run pawl control until both engaged. Action complete - 1, Run again - 0, Error < 0
        ret_val = engageBoth(RUN_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
            state = ABORT;
        } else if (ret_val == 1){
            state = TURN_MOTOR_OFF;
        }
        break;

    case TURN_MOTOR_OFF:

        turnOffMotor();
        state = SEND_TO_UCCM;
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
        uccm_send("%c%c\r\n", cur_cmd->msg >> 8, cur_cmd->msg & 0xFF);

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
    unsigned int pos, setpos;
    int err;
    unsigned int next_state = IDLE;

    // The first bytes xxxx_xxx0 ignoring the least significant bit is the identifier
    switch((int) (msg[0]) >> 1) {

    case SETPOS_MSG:

        //-- Byte[0] 0000_000x << 8 + Byte[1] xxxx_xxxx
        setpos = ((int)msg[0] & 0x1) << 8 | msg[1];

        //-- If this action is busy we dont calculate pos and dir but create a busy command instead
        if (!is_busy(SET_POS) && setpos <= 360) {

            err = setDirectionToMove(setpos);

            if (err < 0) {
                //-- TODO: Do something?
                next_state = ABORT;
            } else {
                next_state = TURN_MOTOR_ON;
            }

            setpos |= SETPOS_MSG << 9;

        } else {

            //-- If we are here it is either busy or pos > 360. new_command automaticaly sets uccm_msg
            //-- for busy therefore we set pos to an error msg
            setpos = 0xFF;
            next_state = SEND_TO_UCCM;
        }

        //-- Sets uccm_msg - pos. If busy then the command is automatically set to busy
        cur_cmd = new_command(SET_POS, setpos);
        break;

    case QUERYPOS_MSG:

        pos = getCurrentCachedPosition();

        //-- data1: 0, data2: 0, uccm_msg: pot. If busy then command is set to busy type
        cur_cmd = new_command(QUERY_POS, pos);

        next_state = SEND_TO_UCCM;
        break;

    case STOPLOCK_MSG:

        //-- We need to clear all current commands that are running so we do not return to them after
        clear_all_commands();

        cur_cmd = new_command(STOPLOCK, STOPLOCK_MSG << 9);

        next_state = ABORT;
        break;

    case ALIVE_MSG:

        cur_cmd = new_command(ALIVE, 0x5555);

        next_state = SEND_TO_UCCM;
        break;

    default:            // UNDEF

        cur_cmd = new_command(UNDEF, 0xFF00);

        next_state = SEND_TO_UCCM;
        break;
    }
    return next_state;
}

/*
 * Halts the main motor and Engages Pawl
 */
static int abort_action(void) {
   int err;

   //-- Halt motor operation
  stopMainMotor();

   //-- Have motor on before we move pawls
   turnOnMotor();

    //-- Engage pawl
   err = engageBoth(INIT_PAWL);
   if (err < 0) {
       return -1;
   }

   //-- Perform this function until either error or returns 1
   do {
       err = engageBoth(RUN_PAWL);

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

