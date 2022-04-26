/*
 * statemachine.c
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */
#include <msp430.h>
#include "statemachine.h"

unsigned int state = WAIT;

void handle_commands(void) {
    while (1) {

        //-- If rx_ready we have an incoming msg

        switch(state) {
        case WAIT:
            break;
        case DECODE:
            //-- Decode msg

            //-- Create a new command

            //-- Figure out which direction
            break;
        case
        }
    }
}
