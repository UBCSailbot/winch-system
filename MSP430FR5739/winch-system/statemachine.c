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


void handle_commands(void) {
    char rx_msg[RXBUF_LEN] = "";

    while (1) {

        if (isReady()) {

            getMsg(rx_msg);

            add_new_command((unsigned int) rx_msg[0] | ((unsigned int) rx_msg[1]) << 8);

            clearReady();
        }


        statemachine();

        //-- 100 Hz
        __delay_cycles(10000);
    }
}


static void statemachine(void) {
    int ret_val = 0;
    t_state next_state = IDLE;
    unsigned int tx_msg;

    switch(get_current_command_state()) {

    case IDLE:
        //--  Idle wait
        break;

    case DECODE:
        // Decode message and find next state
        next_state = decode_msg();
        break;

    case TURN_MOTOR_ON:
        turnOnMotor();
        next_state = START_PAWL;
        break;

    //-- SET_POS states
    case START_PAWL:
        //-- Send the direction data and initialize the PAWL
        ret_val = move_pawl(INIT_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            set_current_tx_msg(0xFF);
            next_state = ABORT;
        } else {
            next_state = WAIT_PAWL;
        }
        break;

    case WAIT_PAWL:
        //-- Run pawl until action is complete. Action complete - 1, Run again - 0, Error < 0
        ret_val = move_pawl(RUN_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            set_current_tx_msg(0xFF);
            next_state = ABORT;
        } else if (ret_val == 1){
            next_state = START_MOTOR;
        }
        break;

    case START_MOTOR:
        //-- Initialize motor functions
        ret_val = setMainMotorPosition(INIT_MMOTOR);
        if (ret_val < 0) {
            //-- ERROR
            set_current_tx_msg(0xFF);
            next_state = ABORT;
        } else{
            next_state = WAIT_MOTOR;
        }
        break;

    case WAIT_MOTOR:
        //-- Run motor until action is complete. Action complete - 1, Run again - 0, Error < 0
        ret_val = setMainMotorPosition(RUN_MMOTOR);
        if (ret_val < 0) {
            //-- ERROR
            set_current_tx_msg(0xFF);
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
        ret_val = engageBoth(INIT_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            set_current_tx_msg(0xFF);
            next_state = ABORT;
        } else {
            next_state = WAIT_ENGAGE_PAWL;
        }
        break;

    case WAIT_ENGAGE_PAWL:
        //-- Run pawl control until both engaged. Action complete - 1, Run again - 0, Error < 0
        ret_val = engageBoth(RUN_PAWL);
        if (ret_val < 0) {
            //-- ERROR
            set_current_tx_msg(0xFF);
            next_state = ABORT;
        } else if (ret_val == 1){
            next_state = TURN_MOTOR_OFF;
        }
        break;

    case TURN_MOTOR_OFF:
        turnOffMotor();
        next_state = SEND_TO_UCCM;
        break;

    case ABORT:
        //-- Primary action to stop winch functionality
        if (abort_action() < 0) {
            //-- Perform a PUC reset?
        }

        next_state = SEND_TO_UCCM;
        break;

    case SEND_TO_UCCM:
        tx_msg = get_current_tx_msg();

        //-- Send message to the UCCM
        uccm_send("%c%c\r\n", tx_msg >> 8, tx_msg & 0xFF);

        //-- Removes current command from the list
        end_command();

        next_state = IDLE;

        break;
    }

    set_current_command_state(next_state);
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
t_state decode_msg(void) {
    unsigned int pos, setpos;
    int err;
    t_state next_state = IDLE;
    unsigned int rx_msg = get_current_rx_msg();

    // The first bytes xxxx_xxx0 ignoring the least significant bit is the identifier
    switch((rx_msg & 0xFF) >> 1) {

    case SETPOS_MSG:

        //-- Byte[0] 0000_000x << 8 + Byte[1] xxxx_xxxx
        setpos = (rx_msg & 0x1) << 8 | (rx_msg >> 8);

        if (setpos > 360) {
            next_state = set_current_command(UNDEF, 0xFF00);
            break;
        }

        err = setDirectionToMove(setpos);

        if (err < 0) {
            next_state = set_current_command(STOPLOCK, 0xFF);
            break;
        }

        next_state = set_current_command(SET_POS, setpos);
        break;

    case QUERYPOS_MSG:

        pos = getCurrentCachedPosition();

        next_state = set_current_command(QUERY_POS, pos);
        break;

    case STOPLOCK_MSG:

        //-- We need to clear all other current commands that are running so we do not return to them after
        clear_all_other_commands();

        next_state = set_current_command(STOPLOCK, STOPLOCK_MSG << 9);
        break;

    case ALIVE_MSG:

        next_state = set_current_command(ALIVE, 0x5555);
        break;

    default:            // UNDEF

        next_state = set_current_command(UNDEF, 0xFF00);
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
