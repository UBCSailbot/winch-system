#include <msp430.h>
#include "debug.h"
#include <stdio.h>
#include <stdarg.h>

/*
 * debug.c
 *
 *  Created on: Mar. 28, 2022
 *      Author: mlokh
 */

extern int rx_ready;
char control_char = 0;

void dbg_printf(const char *format, ...) {
    va_list args;
    char str[50] = "";

    va_start(args, format);

    vsprintf(str, format, args);

    putString(str);

    va_end(args);
}

void init_UART_DBG(void) {
    UCA1CTLW0 |= UCSWRST + UCSSEL__SMCLK;   // Initially set UART A1 to reset and select SMCLK 1Mhz clk

    UCA1BRW = 0x6;
    UCA1MCTLW |= UCOS16 + 0x2080; // config for baud 9600 with 1Mhz clk. OCOS16 - 1, UCBRF = 0x80 (Lower Byte) and UCBRS = 0x20 (Upper Byte)

    //-- Port 2, pin 6 USBTX and 5 USBRX
    P2SEL1 |= BIT5 + BIT6;       // Pin 5 and 6 are UCA1 TXD and RXD respectively
    P2SEL0 &= ~(BIT5 + BIT6);

    PM5CTL0 &= ~LOCKLPM5;

    UCA1CTLW0 &= ~UCSWRST;  // Clear UART reset

    /* Interrupts */
    UCA1IE |= UCRXIE; // Enable Interrupts for RX
    UCA1IFG &= ~UCRXIFG; // Clear Interrupt flag for RX
}

void putString(char* message) {
    while (*message != '\0') {
        while(!(UCA1IFG & UCTXIFG));

        UCA1TXBUF = *message; // Put character in buffer
        message++;
    }
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
    char c = 0;
    switch(__even_in_range(UCA1IV,18)) {
    case 0x00: // No interrupts
        break;
    case 0x02: // Vector 2: UCRXIFG
        if (!rx_ready) {
            c = UCA1RXBUF;
            rx_ready = 1;
            control_char = c;
            UCA1TXBUF = c;
        }

        break;
    case 0x03: // Vector 4: UCTXIFG
        break;
    default:
        break;
    }
}



