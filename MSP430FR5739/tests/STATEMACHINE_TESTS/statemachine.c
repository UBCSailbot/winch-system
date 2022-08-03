/*
 * statemachine.c
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */
#include <msp430.h>
#include "statemachine.h"
#include "commands.h"
#include "debug.h"
#include "motor.h"
//-- #include "uart.h" in statemachine.h
unsigned int wait_count = 0;

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
//
//        if (++wait_count == 1000) {
//            wait_count = 0;
//            print_cmd_list();
//        }

        //-- 100 Hz
        __delay_cycles(10000);
    }
}

static void statemachine(void) {
    t_ret_code ret_val = COMPLETE;
    t_state current_state, next_state = IDLE;
    unsigned int tx_msg;
    unsigned int rx_msg;
    unsigned int pos, setpos;

    switch(current_state = get_current_command_state()) {

    case IDLE:
        /*  DO NOTHING */
        // print debug idle
        // Turn off cpu

        //V_PRINTF("IDLE")

        LPM0;
        break;

    case DECODE:
        V_PRINTF("DECODE -> ")
        next_state = decode_msg();

        goto SKIP_GET_NEXT_STATE;

    case SET_DIRECTION:
        V_PRINTF("SET_DIRECTION -> ")

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

        break;

    case TURN_MOTOR_ON:
        V_PRINTF("TURN_MOTOR_ON -> ")
        break;

    //-- SET_POS --//
    case START_PAWL:
        V_PRINTF("START_PAWL -> ")
        break;

    case WAIT_PAWL:
        V_PRINTF("WAIT_PAWL -> ")
        break;

    case START_MOTOR:
        V_PRINTF("START_MOTOR -> ")
        //ret_val = setMainMotorPosition(INIT_MMOTOR);
        break;

    case WAIT_MOTOR:
        V_PRINTF("WAIT_MOTOR -> ")
        //ret_val = setMainMotorPosition(RUN_MMOTOR);
        break;

    case START_ENGAGE_PAWL:
        V_PRINTF("START_ENGAGE_PAWL -> ")
        break;

    case WAIT_ENGAGE_PAWL:
        V_PRINTF("WAIT_ENGAGE_PAWL -> ")
        break;

    case TURN_MOTOR_OFF:
        V_PRINTF("TURN_MOTOR_OFF -> ")
        break;

    case ERROR_STATE:
        V_PRINTF("ERROR_STATE -> ")
        set_current_tx_msg(0xFF);
        break;

    case ABORT:
        V_PRINTF("ABORT! -> ")
        break;

    case GET_POSITION:
        pos = getCurrentCachedPosition();
        set_current_tx_msg(pos);

        V_PRINTF("GET_POSITION (%d) -> ", pos)
        break;

    case SEND_TO_UCCM:

        V_PRINTF("SEND_TO_UCCM \r\n")

        tx_msg = get_current_tx_msg();

        //-- Send message to the UCCM
        uccm_send("%c%c\r\n", tx_msg >> 8, tx_msg & 0xFF);

        //-- Removes current command from the list
        end_command();

        break;
    }

    next_state = get_next_state(current_state, ret_val);

SKIP_GET_NEXT_STATE:
    V_PRINTF("nxt_state: %d", next_state)
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

        next_state = set_current_command(STOPLOCK, STOPLOCK_MSG << 9);
        //-- We need to clear all other current commands that are running so we do not return to them after
        clear_all_other_commands();

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

t_state get_next_state(t_state current_state, t_ret_code returned_code) {
    t_state next_state = IDLE;

    if (current_state < MAX_STATE && returned_code < MAX_RET_CODE) {
        next_state = next_state_lookup_table[current_state][returned_code];
    }

    return next_state;
}
