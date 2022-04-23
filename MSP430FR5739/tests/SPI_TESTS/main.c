#include <msp430.h> 
#include <stdio.h>
#include "spi.h"
#include "debug.h"

//-- Types of tests
#define POT_T 1
#define CONF_T 2

//-- CONTROL
#define TEST_SEL POT_T
#define VERBOSE 1

void test_pot(void);
void test_hallConfig(void);

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	init_spi();
	init_UART_DBG();

	switch(TEST_SEL) {
	case POT_T:
	    test_pot();
	    break;
	case CONF_T:
	    test_hallConfig();
	    break;
	}

	for(;;);
}


//-- TESTS --

void test_pot(void) {

    unsigned int pot_data;
    char str[50] = "";

    while (1) {
        receive_potentiometer(&pot_data);

        if (VERBOSE) {
            sprintf(str, "Pot_data: %d\n\r\0", pot_data);
            putString(str);
        }

        __delay_cycles(2500000);
    }

}

void test_hallConfig(void) {
    int return_data;
    char str[50] = "";

    while (1) {
        return_data = configHall(AIN0_CONF); /* 0x14AB */

        if (VERBOSE) {
            sprintf(str, "SUCCESS: %s\n\r", return_data ? "NO" : "YES");
            putString(str);
        }

    }
}

