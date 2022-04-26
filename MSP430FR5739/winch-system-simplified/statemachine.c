/*
 * statemachine.c
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */
#include "statemachine.h"
#include "uart.h"

typedef struct machine {
    int state = WAIT;
    int data = 0;
} t_machine;

int busy = 0;

void handle_state(void) {
    char msg[2];
    t_machine machine;
    int pos;

    while (1) {
        if (busy && isReady()) putString("Busy\r\n");
        switch(machine.state) {
        case WAIT:
            //-- If ready flag received
            if (isReady()) {
                getCommand(msg);
                machine.state = DECODE;
                clearReady();
            }
            break;
        case DECODE:
            decode_msg(msg, &pos);

            break;
        case PAWL:
            break;
        case MOTOR:
            break;
        case NOTIFY_UCCM:
            break;

        }
    }
}

int decode_msg(char msg[2], int * data) {

    // The first byte contains the identifier (shifts right to remove unused first bit)
    switch((int) (msg[0] >> 1)) {
    case 0b1000:
        //-- TODO: verify this
        *data = ((int)msg[0] & 0x1) << 8 | msg[1];
        return SET_POS;
        break;
    case
    }
    return 0;
}


