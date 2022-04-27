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
#include "uart.h"
#include "motor.h"
#include "pawl.h"

unsigned int state = WAIT;
t_cmd * cur_cmd;

void handle_commands(void) {
    char msg[RXBUF_LEN] = "";
    int ret_val = 0;

    while (1) {

        //-- If rx_ready we have an incoming msg
        if (isReady()) {

            //-- If the maximum active command threshold is not reached
            if (!max_active_reached()) {
                getMsg(msg);

                //-- NEED TO DO THIS SO WE CAN RETURN
                save_current_state(state);

                state = DECODE;
            }

            //-- We ignore the rx_ready flag if max active threshold has been reached
            clearReady();
        }

        switch(state) {
        case IDLE:
            //--  Idle wait
            break;
        case DECODE:
            // Decode message and find next state
            state = decode_msg(msg);
            break;

        //-- SET_POS states
        case START_PAWL:
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
            ret_val = setMainMotorPosition(cur_cmd->data1, cur_cmd->data2, INIT_MMOTOR);
            if (ret_val < 0) {
                //-- ERROR
                cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
                state = ABORT;
            } else{
                state = WAIT_MOTOR;
            }
            break;

        case WAIT_MOTOR:
            ret_val = setMainMotorPosition(cur_cmd->data1, cur_cmd->data2, RUN_MMOTOR);
            if (ret_val < 0) {
                //-- ERROR
                cur_cmd->msg = 0xFF;    //TODO: Build a error lookup
                state = ABORT;
            } else if (ret_val == 1) {
                state = ENGAGE_PAWL_1;
            }
            break;

        case ENGAGE_PAWL_1:
            break;

        case ENGAGE_PAWL_2:
            break;

        case ABORT:
            break;

        case SEND_TO_UCCM:
            //-- Send message to the UCCM
            uccm_send("%x\r\n\0", cur_cmd->msg);

            //-- Removes command from the list
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
}

/*
 *  Decodes the message from the UCCM and returns the next state
 */
int decode_msg(char msg[2]) {

    unsigned int pos, dir, pot;
    unsigned int next_state = IDLE;

    // The first byte contains the identifier (shifts right to remove unused first bit)
    switch((int) (msg[0] >> 1)) {

    case 0b1000:            // SET_POS
        if (!is_busy(SET_POS)) {

            //-- TODO: verify this
            pos = ((int)msg[0] & 0x1) << 8 | msg[1];

            //-- Call receive potentiometer and figure out direction
            dir = getDirection(pos);

            next_state = START_PAWL;
        } else {
            //-- If it is busy then next state is send to UCCM
            next_state = SEND_TO_UCCM;
        }

        //-- Sets data1 - pos data2 - dir and uccm_msg - pos. If busy then command set to busy
        cur_cmd = new_command(SET_POS, pos, dir, pos);
        break;

    case 0b0010:        // QUERY_POS
        //-- Receive pot data from pot
        receive_potentiometer(&pot);

        //-- data1: pot, data2: 0, uccm_msg: pot. If busy then command is set to busy type
        cur_cmd = new_command(QUERY_POS, pot, 0,pot);

        next_state = SEND_TO_UCCM;
        break;

    case 0b0011:        // STOPLOCK
        cur_cmd = new_command(STOPLOCK, 0, 0,0x6);

        next_state = ABORT;
        break;
    case 0b0100:        // ALIVE
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

