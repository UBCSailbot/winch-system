#include <msp430.h> 
#include "pawl.h"

cur_direction = 0;
/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	return 0;
}
