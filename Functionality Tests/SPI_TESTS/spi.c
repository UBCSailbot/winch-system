/*
 * spi.c
 *
 *  Created on: Apr. 3, 2022
 *      Author: mlokh
 */

#include <msp430.h>
#include <stdio.h>
#include "spi.h"
#include "debug.h"


/**
 * Using the UCA0 peripheral for SPI in 3 pin mode and 2 GPIO pins for CS
 * CS_POT -> P3.6
 * CS_HALL -> P3.5  |
 *                  |_ PAWL_LEFT (AIN0)
 *                  |_ CAM (AIN1)
 *                  |_ PAWL_RIGHT (AIN2)
 */
void init_spi(void) {

    //-- Initializing all eUSCI registers

    UCA0CTLW0 |= UCSWRST;       // Enter reset mode
    UCA0CTLW0 |= UCSSEL__SMCLK; // Set clk to 1 MHz
    UCA0CTLW0 |= UCSYNC;        // Synchronous mode (SPI)
    UCA0CTLW0 |= UCMST;         // Master mode
    UCA0CTLW0 |= UCMSB;         // MSB first

    UCA0BRW |= 0x03;            // 1 Mhz/2

    //-- Port configuration

    /* P2.1 for UCA0SOMI */
    P2SEL1 |= BIT1;
    P2SEL0 &= ~BIT1;

    /* P2.0 for UCA0SIMO */
    P2SEL1 |= BIT0;
    P2SEL0 &= ~BIT0;

    /* P1.5 for UCA0CLK */
    P1SEL1 |= BIT5;
    P1SEL0 &= ~BIT5;

    /* P3.6 CS_POT selects potentiometer */
    P3DIR |= CS_POT;
    P3OUT |= CS_POT;    // Active low

    /* P3.5 CS_HALL selects hallsensor */
    P3DIR |= CS_HALL;
    P3OUT |= CS_HALL;   // Active low


    UCA0CTLW0 &= ~UCSWRST;       // Move out of reset mode before enabling interrupts
}


/* Allows to receive Hallsensor data from either of the sensors.
 * If a certain sensor data is not needed pass in a NULL
 */
void receive_hallsensors(int* pawl_left, int* cam, int* pawl_right) {

    if (pawl_left != NULL) {
        if (configHall(AIN0_CONF) < 0) {
            *pawl_left = -1;
        } else {
            //-- Receive Hall sensor data
            *pawl_left = spi_io(0, 2, CS_HALL);
        }
    }

    if (cam != NULL) {
        if (configHall(AIN1_CONF) < 0) {
            *cam = -1;
        } else {
            //-- Receive Hall sensor data
            *cam = spi_io(0, 2, CS_HALL);
        }
     }

    if (pawl_right != NULL) {
        if (configHall(AIN2_CONF) < 0) {
            *pawl_right = -1;
        } else {
            //-- Receive Hall sensor data
            *pawl_right = spi_io(0, 2, CS_HALL);
        }
     }
}

/*
 * Allows to receive Potentiometer data
 */

void receive_potentiometer(unsigned int* pot_data) {
    *pot_data = spi_io(0x55, 2, CS_POT);
}


/**
 * This function configures the ADC and confirms if config has been set
 *
 * return 0 if set and -1 otherwise
 */
int configHall(unsigned int config) {
    unsigned long returned_config = 0;
    int attempts = 0;


    do {

        P3OUT &= ~CS_HALL;
        //-- We receive the configuration value in the most significant 2 bytes
        spi_io(config, 2, CS_HALL);

        // Can only receive 2 bytes (2 byte registers on MSP430

        returned_config = spi_io(config, 2, CS_HALL);

        P3OUT |= CS_HALL;
        //-- bit shift 2 bytes to the right to get configuration value
        //returned_config >>= 16;

        if (++attempts > 4) return -1;

        __delay_cycles(250000);
    } while (returned_config != config);

    return 0;
}


/**
 * Performs IO operation through SPI
 *
 * data - to be sent through SPI
 * bytes - number of bytes being sent
 * enable - the slave device being communicated with in P3
 */
static unsigned long spi_io(unsigned int data, int bytes, int chipSel) {
    unsigned int rx_buf, offset;
    unsigned int rx_data = 0;
    unsigned int tmp;
    int i = 1;

    //P3OUT &= ~chipSel;

    for (i = 1; i <= bytes; i++) {
        offset =  8*(bytes - i);

        while (!(UCA0IFG & UCTXIFG));
        tmp = (data >> offset) & 0xFF;

        //-- Transmit MSB first
        UCA0TXBUF = tmp;

        while(!(UCA0IFG & UCRXIFG));

        //-- Receive MSB first
        rx_buf = UCA0RXBUF;
        rx_data += rx_buf << offset;
    }

    while (UCBUSY & UCA0STATW);     // Wait until not busy

    rx_buf = UCA0RXBUF;             // Receive dummy byte

    //P3OUT |= chipSel;

    return rx_data;
}



