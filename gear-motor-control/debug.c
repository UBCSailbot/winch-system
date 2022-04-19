#include <msp430.h>
#include "debug.h"

/*
 * debug.c
 *
 *  Created on: Mar. 28, 2022
 *      Author: mlokh
 */

void init_UART_DBG(void) {
    UCA1CTLW0 |= UCSWRST + UCSSEL__SMCLK;   // Initially set UART A1 to reset and select SMCLK 1Mhz clk

    UCA1BRW = 0x6;
    UCA1MCTLW |= UCOS16 + 0x2080; // config for baud 9600 with 1Mhz clk. OCOS16 - 1, UCBRF = 0x80 (Lower Byte) and UCBRS = 0x20 (Upper Byte)

    //-- Port 2, pin 6 USBTX and 5 USBRX
    P2SEL1 |= BIT5 + BIT6;       // Pin 5 and 6 are UCA1 TXD and RXD respectively
    P2SEL0 &= ~(BIT5 + BIT6);

    PM5CTL0 &= ~LOCKLPM5;

    UCA1CTLW0 &= ~UCSWRST;  // Clear UART reset
}

void putString(char* message) {
    while (*message != '\0') {
        while(!(UCA1IFG & UCTXIFG));

        UCA1TXBUF = *message; // Put character in buffer
        message++;
    }
}



