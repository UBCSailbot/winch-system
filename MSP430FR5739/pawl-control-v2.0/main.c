#include <msp430.h> 
#include "pawl.h"
#include "spi.h"
#include "gearmotor.h"
#include "debug.h"
#include "motor.h"
#include "uart.h"

int cur_direction = -1;
extern char control_char;

void init(void);
/**
 * main.c
 * Frequencies for Control_board_v.2
 *  DCOCLK = 8MHz (default set on MSP430)
 *  MCLK = 1MHz (DCOCLK derived, 1/8 divider)
 *  ACLK = 10kHz (VLO derived, automatically transitions from XT1 in LF mode
 *                  which requires an external oscillator; none is present
 *                  in the Control_board_v.2, thereby triggering fault and
 *                  switching ACLK to VLO, see 3.2.7 CS Module Fail-Safe
 *                  Operation)
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	init();

	int err;
	V_PRINTF("\r\nSTART\r\n");
	while(1) {
	    if (isReady()) {
	        V_PRINTF("RUN\r\n");
	        clearReady();
	        switch((int)control_char-48) {  // converts ASCII numerical to int numerical
	        case 1:
	            cur_direction = CLOCKWISE;
	            break;
	        case 2:
	            cur_direction = ANTICLOCKWISE;
	            break;
	        case 3:
	            cur_direction = REST;
	            break;
	        case 4:
	            cur_direction = ROTATE_CW;
	            break;

	        default:
	            continue;
	        }

	        if ((err = move_pawl()) == 0) {
	                V_PRINTF("\r\nSUCCESS\r\n");
	            }else {
	                V_PRINTF("\r\nFAIL err: %d \r\n", err);
	        }
	        clearReady();
	    }
	}
}


void init(void) {


    init_spi();
    init_gearmotor();
    init_uart();
    init_Main_Motor();

    __enable_interrupt();
}
