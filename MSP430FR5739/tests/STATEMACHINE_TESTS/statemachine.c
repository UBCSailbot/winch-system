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
    int next_state;

    switch(state) {

    case IDLE:
        /*  DO NOTHING */
        // print debug idle
        // Turn off cpu

        V_PRINTF("IDLE")
        __low_power_mode_4();
        break;

    case DECODE:
        next_state = decode_msg(msg);
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

        next_state = WAIT_MOTOR;
        break;

    case WAIT_MOTOR:

        V_PRINTF("WAIT_MOTOR -> ")

        next_state = START_ENGAGE_PAWL;
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

        //-- Send message to the UCCM
        uccm_send("%c%c\r\n", cur_cmd->msg >> 8, cur_cmd->msg & 0xFF);

        //-- Removes current command from the list
        end_command();

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

    state = next_state;
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
    unsigned int next_state = IDLE;

    // The first bytes xxxx_xxx0 ignoring the least significant bit is the identifier
    switch((int) (msg[0]) >> 1) {

    case SETPOS_MSG:

        //-- Byte[0] 0000_000x << 8 + Byte[1] xxxx_xxxx
        setpos = 180;

        //-- If this action is busy we dont calculate pos and dir but create a busy command instead
        if (!is_busy(SET_POS) && setpos <= 360) {

            setpos |= SETPOS_MSG << 9;
            next_state = TURN_MOTOR_ON;

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

        pos = 180;

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
