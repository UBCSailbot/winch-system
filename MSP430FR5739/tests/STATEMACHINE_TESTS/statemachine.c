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
    t_state next_state = IDLE;
    unsigned int tx_msg;
    int ret_val;

    switch(get_current_command_state()) {

    case IDLE:
        /*  DO NOTHING */
        // print debug idle
        // Turn off cpu

        V_PRINTF("IDLE")

        LPM4;
        break;

    case DECODE:
        next_state = decode_msg();
        break;

    case TURN_MOTOR_ON:
        V_PRINTF("TURN_MOTOR_ON -> ")
        next_state = START_PAWL;
        break;

    //-- SET_POS --//
    case START_PAWL:

        V_PRINTF("START_PAWL -> ")

        next_state = WAIT_PAWL;
        break;

    case WAIT_PAWL:

        V_PRINTF("WAIT_PAWL -> ")

        next_state = START_MOTOR;
        break;

    case START_MOTOR:

        V_PRINTF("START_MOTOR -> ")

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

        V_PRINTF("WAIT_MOTOR -> ")

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
        } else {
            next_state = WAIT_MOTOR;
        }

        break;

    case START_ENGAGE_PAWL:

        V_PRINTF("START_ENGAGE_PAWL -> ")

        next_state = WAIT_ENGAGE_PAWL;
        break;

    case WAIT_ENGAGE_PAWL:

        V_PRINTF("WAIT_ENGAGE_PAWL -> ")

        next_state = TURN_MOTOR_OFF;
        break;

    case TURN_MOTOR_OFF:

        V_PRINTF("TURN_MOTOR_OFF -> ")
        next_state = SEND_TO_UCCM;
        break;

    case ABORT:

        V_PRINTF("ABORT! -> ")

        next_state = SEND_TO_UCCM;
        break;

    case SEND_TO_UCCM:

        V_PRINTF("SEND_TO_UCCM \r\n")

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
        V_PRINTF("SETPOS ->")

        //-- Byte[0] 0000_000x << 8 + Byte[1] xxxx_xxxx
        setpos = (rx_msg & 0x1) << 8 | (rx_msg >> 8);

        V_PRINTF(" (setpos: %d) ", setpos)
        if (setpos > 360) {
            next_state = set_current_command(UNDEF, 0xFF00);
            break;
        }

        if (!is_busy(SET_POS)) {
            V_PRINTF("Setdir")
            err = setDirectionToMove(setpos);

            if (err < 0) {
                next_state = set_current_command(STOPLOCK, 0xFF);
                break;
            }
        }

        next_state = set_current_command(SET_POS, setpos);
        break;

    case QUERYPOS_MSG:
        V_PRINTF("QUERYPOS ->")

        pos = getCurrentCachedPosition();

        next_state = set_current_command(QUERY_POS, pos);
        break;

    case STOPLOCK_MSG:
        V_PRINTF("STOP&LOCK ->")
        //-- We need to clear all other current commands that are running so we do not return to them after
        clear_all_other_commands();

        next_state = set_current_command(STOPLOCK, STOPLOCK_MSG << 9);
        break;

    case ALIVE_MSG:
        V_PRINTF("ALIVE ->")

        next_state = set_current_command(ALIVE, 0x5555);
        break;

    default:            // UNDEF

        V_PRINTF("UNDEF ->")
        next_state = set_current_command(UNDEF, 0xFF00);
        break;
    }

    return next_state;
}
