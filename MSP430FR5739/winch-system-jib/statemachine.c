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

static const t_state next_state_lookup_table[MAX_STATE][MAX_RET_CODE] =
{
 // RUN_AGAIN       COMPLETE            ERROR           RESTART

 //---- IDLE ----
 {IDLE,              IDLE,               ERROR_STATE,          IDLE},

 //---- DECODE ----
 {DECODE,            IDLE,               ERROR_STATE,         ABORT},

 //---- SET_DIRECTION ----
 {SET_DIRECTION,     TURN_MOTOR_ON,      ERROR_STATE,         ABORT},

 //---- TURN_MOTOR_ON ----
 {TURN_MOTOR_ON,     START_PAWL,         ERROR_STATE,         ABORT},

 //---- START_PAWL ----
 {START_PAWL,        WAIT_PAWL,          ERROR_STATE,         ABORT},

 //---- WAIT_PAWL ----
 {WAIT_PAWL,         START_MOTOR,        ERROR_STATE,         ABORT},

 //---- START_MOTOR ----
 {START_MOTOR,       WAIT_MOTOR,         ERROR_STATE,         ABORT},

 //---- WAIT_MOTOR ----
 {WAIT_MOTOR,        START_ENGAGE_PAWL,  ERROR_STATE,         START_PAWL},

 //---- START_ENGAGE_PAWL ----
 {START_ENGAGE_PAWL, WAIT_ENGAGE_PAWL,   ERROR_STATE,         ABORT},

 //---- WAIT_ENGAGE_PAWL ----
 {WAIT_ENGAGE_PAWL,  TURN_MOTOR_OFF,     ERROR_STATE,         ABORT},

 //---- TURN_MOTOR_OFF ----
 {TURN_MOTOR_OFF,    SEND_TO_UCCM,       ERROR_STATE,         ABORT},

 //---- GET_POSITION ----
 {GET_POSITION,      SEND_TO_UCCM,       ERROR_STATE,         ABORT},

 //---- ABORT ----
 {ABORT,             SEND_TO_UCCM,       ERROR_STATE,         ABORT},

 //---- SEND_TO_UCCM ----
 {SEND_TO_UCCM,      IDLE,               ERROR_STATE,         ABORT},

 //---- ERROR_STATE ----
 {ERROR_STATE,      ABORT,               ABORT,               ABORT}
};


void handle_commands(void) {
    char rx_msg[RXBUF_LEN] = "";

    while (1) {

        if (isReady()) {

            getMsg(rx_msg);

            add_new_command((unsigned int) rx_msg[0] | ((unsigned int) rx_msg[1]) << 8);

            clearReady();
        }

        statemachine();

        //-- 1000 Hz
        __delay_cycles(1000);
    }
}


static void statemachine(void) {
    t_ret_code ret_val = COMPLETE;
    t_state current_state, next_state = IDLE;
    unsigned int tx_msg;
    unsigned int rx_msg;
    unsigned int pos, setpos;

    switch( current_state = get_current_command_state()) {

    case IDLE:
        //--  Idle turn off cpu
        LPM0;
        break;

    case DECODE:
        // Decode message and find next state
        next_state = decode_msg();

        goto SKIP_NEXT_STATE_LOOKUP;

    case SET_DIRECTION:
        rx_msg = get_current_rx_msg();

        //-- Byte[0] 0000_000x << 8 + Byte[1] xxxx_xxxx
        setpos = (rx_msg & 0x1) << 8 | (rx_msg >> 8);

        if (setpos > 360) {
            ret_val = ERROR;
            break;
        }

        if (setDirectionToMove(setpos) < 0) {
            ret_val = ERROR;
            break;
        }

        set_current_tx_msg(setpos | (SETPOS_MSG << 9));
        break;

    case TURN_MOTOR_ON:
        turnOnMotor();
        break;

    //-- SET_POS states
    case START_PAWL:
        ret_val = move_pawl(INIT_PAWL);
        break;

    case WAIT_PAWL:
        ret_val = move_pawl(RUN_PAWL);
        break;

    case START_MOTOR:
        ret_val = setMainMotorPosition(INIT_MMOTOR);
        break;

    case WAIT_MOTOR:
        ret_val = setMainMotorPosition(RUN_MMOTOR);
        break;

    case START_ENGAGE_PAWL:
        ret_val = engageBoth(INIT_PAWL);
        break;

    case WAIT_ENGAGE_PAWL:
        ret_val = engageBoth(RUN_PAWL);
        break;

    case TURN_MOTOR_OFF:
        turnOffMotor();
        break;

    case ERROR_STATE:
        set_current_tx_msg(ERROR_MSG);
        break;

    case ABORT:
        V_PRINTF("ABORT");
        //-- Primary action to stop winch functionality
        if (abort_action() < 0) {
            //-- Perform a PUC reset?
            V_PRINTF("ABORT ERR\r\n");
        }
        break;

    case GET_POSITION:
        pos = getCurrentCachedPosition();
        set_current_tx_msg(pos);
        break;

    case SEND_TO_UCCM:
        tx_msg = get_current_tx_msg();

        //-- Send message to the UCCM
        uccm_send("%c%c\r\n", tx_msg >> 8, tx_msg & 0xFF);

        //-- Removes current command from the list
        end_command();
        break;
    }

    next_state = get_next_state(current_state, ret_val);

SKIP_NEXT_STATE_LOOKUP:

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
    t_state next_state = IDLE;
    unsigned int rx_msg = get_current_rx_msg();

    // The first bytes xxxx_xxx0 ignoring the least significant bit is the identifier
    switch((rx_msg & 0xFF) >> 1) {

    case SETPOS_MSG:

        next_state = set_current_command(SET_POS, 0x00);
        break;

    case QUERYPOS_MSG:

        next_state = set_current_command(QUERY_POS, 0x00);
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
   int ret;

   //-- Halt motor operation
  stopMainMotor();

   //-- Have motor on before we move pawls
   turnOnMotor();

    //-- Engage pawl
   ret = engageBoth(INIT_PAWL);
   if (ret == ERROR) {
       return -1;
   }

   //-- Perform this function until either error or returns 1
   do {
       ret = engageBoth(RUN_PAWL);

       //-- If there something is wrong error
       if (ret == ERROR) {
           return -2;
       }

       //-- 100 Hz
       __delay_cycles(10000);
   } while (ret == RUN_AGAIN);


   //-- Turn off power to the main motor
   turnOffMotor();

   return 0;
}

t_state get_next_state(t_state current_state, t_ret_code returned_code) {
    t_state next_state = IDLE;

    if (current_state < MAX_STATE && returned_code < MAX_RET_CODE) {
        next_state = next_state_lookup_table[current_state][returned_code];
    }

    return next_state;
}
