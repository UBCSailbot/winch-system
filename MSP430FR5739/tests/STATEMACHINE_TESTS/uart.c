/*
 * uart.c
 *
 *  Created on: Apr. 21, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart.h"


volatile int rx_flag = 0;
char rxbuf[RXBUF_LEN] = "";
size_t bufpos = 0;
static size_t uart_state  = READ;


void init_uart(void) {
    UCA1CTLW0 |= UCSWRST + UCSSEL__SMCLK;   // Initially set UART A1 to reset and select SMCLK 1Mhz clk

    UCA1BRW = 0x6;
    UCA1MCTLW |= UCOS16 + 0x1180; // config for baud 9600 with 1Mhz clk. OCOS16 - 1, UCBRF = 0x80 (Lower Byte) and UCBRS = 0x11 (Upper Byte)

    //-- Port 2, pin 6 USBTX and 5 USBRX
    P2SEL1 |= BIT5 + BIT6;       // Pin 5 and 6 are UCA1 TXD and RXD respectively
    P2SEL0 &= ~(BIT5 + BIT6);

    PM5CTL0 &= ~LOCKLPM5;

    UCA1CTLW0 &= ~UCSWRST;  // Clear UART reset

    /* Interrupts */
    UCA1IE |= UCRXIE; // Enable Interrupts for RX
    UCA1IFG &= ~UCRXIFG; // Clear Interrupt flag for RX
}

int isReady(void) {
    return rx_flag;
}

void clearReady(void) {
    rx_flag = 0;
}

void getMsg(char* msg) {
    strcpy(msg, rxbuf);
}

void uccm_send(const char *format, ...) {
    va_list args;
    unsigned int num_write;
    char str[MAX_UCCM_SEND] = "";

    va_start(args, format);

    num_write = vsnprintf (str, MAX_UCCM_SEND, format, args);
    str[num_write] = '*';

    putString(str);

    va_end(args);
}

void putString(char* message) {
    unsigned int count = 0;

    while (*message != '*' && count++ < MAX_UCCM_SEND) {
        while(!(UCA1IFG & UCTXIFG));

        UCA1TXBUF = *message; // Put character in buffer
        message++;
    }
}

//-- USCI_A1 interrupt ISR
//#pragma vector = USCI_A1_VECTOR
//__interrupt void USCI_A1_ISR(void) {
//    char received_char = UCA1RXBUF;
//
//    switch(__even_in_range(UCA1IV,18)) {
//    case 0x00: // No interrupts
//        break;
//    case 0x02: // Vector 2: UCRXIFG
//        switch(uart_state) {
//            case PROCESS:
//                if (received_char == '\n' || received_char == '\r') {
//                    // SUCCES as line ends after 2 bytes
//                    rx_flag = 1;
//                    rxbuf[bufpos] = '\0';
//                    LPM0_EXIT;
//                    uart_state = READ;
//                } else {
//                    uart_state = WAIT;
//                }
//
//                bufpos = 0;
//                break;
//
//            case READ:
//
//                if (c == '\n' || c == '\r') {
//                    // Less than 2 bytes - reset buffer position
//                    bufpos = 0;
//                } else {
//                    rxbuf[bufpos] = c;
//                    bufpos++;
//
//                    if (bufpos == RXBUF_LEN) uart_state = PROCESS; // If we reached the end of the buffer Proccess it
//                }
//
//                break;
//
//            case WAIT:
//                // Go to read state when receiving end of character and rx_flag is not set
//                if ((c == '\n' || c == '\r') && !rx_flag) uart_state = READ;
//                break;
//
//            default:
//                break;
//        }
//        break;
//    case 0x04: // Vector 4: UCTXIFG
//        break;
//    default:
//        break;
//    }
//}

//-- TEST UART
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
    char c = 0;
    switch(__even_in_range(UCA1IV,18)) {
    case 0x00: // No interrupts
        break;
    case 0x02: // Vector 2: UCRXIFG

        c = UCA1RXBUF;
        UCA1TXBUF = c;

        break;
    case 0x03: // Vector 4: UCTXIFG
        break;
    default:
        break;
    }
}

