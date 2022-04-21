
// You can map frequently used objects to be able to read or write them
// using In.* and Out.*. Here we map the object 6041:00 as "In.StatusWord".
map U16 StatusWord as input 0x6041:00

map U32 inputReg4 as input 0x60FD:00

// Include the definition of NanoJ functions and symbols
#include "wrapper.h"

#define OFF 0
#define ON 1
#define MASK 0x80000

void start_motor(void);
void stop_motor(void);

// State either OFF or ON
U16 state = 0;


void user()
{

	// Set mode "CLOCK DIRECTION MODE"
	od_write(0x6060, 0x00, -1);
	
	// Set to CLOCK and Direction
	od_write(0x205B, 0x00, 0);
	
	while(1) {
		// Turn on if motor is OFF and input is High
		if ((In.inputReg4 & MASK) && !state) {
			start_motor();
		} else if (!(In.inputReg4 & MASK ) && state) {
			stop_motor();
		} 
		yield();		
	}
	
	stop_motor();

	// Stop the NanoJ program. Without this line, the firmware would
	// call user() again as soon as we return.
	od_write(0x2300, 0x00, 0x0);
}

void start_motor() {
	// Request state "Ready to switch on"
	od_write(0x6040, 0x00, 0x6);

	// Wait until the requested state is reached
	while ( (In.StatusWord & 0xEF) != 0x21) {
		yield(); // Wait for the next cycle (1ms)
	}

	// Request the state "Switched on"
	od_write(0x6040, 0x00, 0x7);

	// Wait until the requested state is reached
	while ( (In.StatusWord & 0xEF) != 0x23) {
		yield();
	}

	// Request the state "Operation enabled"
	od_write(0x6040, 0x00, 0xF);
	
	// Wait until the requested state is reached
	while ( (In.StatusWord & 0xEF) != 0x27) {
		yield();
	}
	
	state = ON;
}

void stop_motor() {
	// Stop the motor
	od_write(0x6040, 0x00, 0x0);
	
	state = OFF;
}
