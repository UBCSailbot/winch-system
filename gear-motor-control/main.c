#include <msp430.h> 
#include "spi.h"
#include "debug.h"
#include <stdio.h>

//-- Port J
#define NFAULT BIT3
#define MODE BIT2
#define PHASE BIT1
#define NSLEEP BIT0

//-- Port 1
#define ENABLE BIT4

/**
 * main.c
 */
void init_motor_driver(void);
void test_motor_driver(void);
void drive_forward(void);

void init_timer(void);
void init_stoptimer(void);


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	init_motor_driver();
	init_UART_DBG();
	init_spi();


    //-- Slow decay mode
    PJOUT |= MODE;

    //-- Direction
    PJOUT &= ~PHASE;

    //-- IC active
    PJOUT |= NSLEEP;


	init_timer();
	init_stoptimer();

	__enable_interrupt();

	int pawl_left, cam, pawl_right;
	char str[150] = "";

	while (1) {
	    receive_hallsensors(&pawl_left, &cam, &pawl_right);
	    sprintf(str, "Pawl_left: %d, Cam: %d, Pawl_right: %d \n\r\0", pawl_left, cam, pawl_right);
	    putString(str);

	    __delay_cycles(250000);
	}

	return 0;
}

void init_motor_driver(void) {

    //-- PJ.3 NFAULT Input
    PJDIR &= ~NFAULT;
    PJREN &= ~NFAULT;    // Pull-up resistor disabled - Pull-up already on the PCB

    //-- PJ.2 MODE Output
    PJDIR |= MODE;
    PJOUT &= ~MODE;

    //-- PJ.1 PHASE Output
    PJDIR |= PHASE;
    PJOUT &= ~PHASE;

    //-- PJ.0 NSLEEP Output
    PJDIR |= NSLEEP;
    PJOUT &= ~NSLEEP;

    //-- P1.4 ENABLE Output
    P1DIR |= ENABLE;
    P1OUT &= ~ENABLE;
}

void init_stoptimer() {
    //-- Timer_A Main timer 1s
    TA1CCR0 = 64000 - 1;

    TA1CCTL0 |= CCIE;

    //-- ACLK (32 khz) / 2, MC - UP, Timer Interrupt enabled
    TA1CTL |= TASSEL_1 + MC_1;
}

void init_timer(void) {

    //-- Port configuration --

    //-- P1.4 ENABLE TB0.1 Output
    P1DIR |= ENABLE;
    P1SEL1 &= ~BIT4;
    P1SEL0 |= BIT4;
    P1OUT |= ENABLE;

    //-- Timer_B Main timer 0.03125s
    TB0CCR0 = 10000 - 1;

    //-- Timer_B compare register - 0.09375s 30% duty cycle
    TB0CCR1 = 10000 - 3000;

    //-- OUTMOD - Toggle/reset
    TB0CCTL1 |= OUTMOD_2;

    //-- ACLK (32 khz), MC - UP
    TB0CTL |= TBSSEL_1 + MC_2;
}

#pragma vector = TIMER1_A0_VECTOR;
__interrupt void TIMER1_A0_ISR (void) {

//    //-- Hault Timer B
//    TB0CTL &=  ~MC_2;
    //TB0CCR0 = 0;
//
//    //-- Hault Timer A
//    TA1CTL &= ~MC_1;
    //TA1CCR0 = 0;


    //-- Clear flag
    TA1CTL &= ~TAIFG;

    //-- Disable Motor
    //P1OUT &= ~ENABLE;
    PJOUT ^= PHASE;
    //-- IC inactive (low power)
    //PJOUT &= ~NSLEEP;
}

void test_motor_driver(void) {

    //-- Forward test **USING DELAYS**
   drive_forward();

}

void drive_forward(void) {
    //-- Slow decay mode
    PJOUT |= MODE;

    //-- Direction
    PJOUT &= ~PHASE;

    //-- IC active
    PJOUT |= NSLEEP;

    //-- Enable motor
    P1OUT &= ~ENABLE;

    //-- Delay for 0.5s
    __delay_cycles(500000);

    //-- Disable Motor
    P1OUT &= ~ENABLE;

    //-- IC inactive (low power)
    PJOUT &= ~NSLEEP;

}
