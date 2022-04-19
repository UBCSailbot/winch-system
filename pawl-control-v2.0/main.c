#include <msp430.h> 
#include "pawl.h"
#include "spi.h"
#include "gearmotor.h"
#include "debug.h"

#define VERBOSE 1
#define V_PRINT(str) while(VERBOSE) {putString(str); break;}

int cur_direction = -1;
int rx_ready = 0;
extern char control_char;

void init(void);
/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	init();

	while(1) {
	    while(rx_ready) {
	        switch(control_char) {
	        case 'c':
	            cur_direction = CLOCKWISE;
	            break;
	        case 'a':
	            cur_direction = ANTICLOCKWISE;
	            break;
	        case 'r':
	            cur_direction = REST;
	            break;
	        }

	        if (move_pawl() == 0) {
	                V_PRINT("SUCCESS\r\n");
	            }else {
	                V_PRINT("FAIL\r\n");
	        }

	        rx_ready = 0;
	    }
	}
}


void init(void) {
    init_spi();
    init_gearmotor();
    init_UART_DBG();

    __enable_interrupt();
}
