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
#include "error.h"


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

    UCA0BRW |= 0x02;            // 1 Mhz/2

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
int receive_hallsensors(unsigned int* pawl_left, int* cam,unsigned int* pawl_right) {
    int err = 0;
    if (pawl_left != NULL) {
        if (!(active_config & AIN0_CONFID)) {
            if (configHall(AIN0_CONF) < 0) {
                err = -1;
                set_error(CONF_HALL_AIN0_ERROR);
                return err;
            } else active_config = AIN0_CONFID;
        }
        P3OUT &= ~CS_HALL;
        //-- Receive Hall sensor data
        *pawl_left = spi_io(0, 2, CS_HALL);
        P3OUT |= CS_HALL;
    }

    if (cam != NULL) {
        if (!(active_config & AIN1_CONFID)) {
            if (configHall(AIN1_CONF) < 0) {
                err = -1;
                set_error(CONF_HALL_AIN1_ERROR);
                return err;
            } else active_config = AIN1_CONFID;
        }
        P3OUT &= ~CS_HALL;
        //-- Receive Hall sensor data
        *cam = spi_io(0, 2, CS_HALL);
        P3OUT |= CS_HALL;
     }

    if (pawl_right != NULL) {
        if (!(active_config & AIN2_CONFID)) {
            if (configHall(AIN2_CONF) < 0) {
                err = -1;
                set_error(CONF_HALL_AIN2_ERROR);
                return err;
            } else active_config = AIN2_CONFID;
        }
        P3OUT &= ~CS_HALL;
        //-- Receive Hall sensor data
        *pawl_right = spi_io(0, 2, CS_HALL);
        P3OUT |= CS_HALL;
     }
    return err;
}

/*
 * Allows to receive Potentiometer data
 * Return -1 if voltage not in the range 500mV to 4500mV
 */
int receive_potentiometer(unsigned int* pot_data) {
    unsigned char tries = 0;
    unsigned int pot_data_value;

    do {
        //-- 10 ms
        if (tries > 0) __delay_cycles(10000);

        P3OUT &= ~CS_POT;
        pot_data_value = spi_io(0x55, 2, CS_POT);
        P3OUT |= CS_POT;
        //V_PRINTF("POT data: %d \r\n", *pot_data);

        if (++tries > MAX_POT_TRIES) {

            if (pot_data_value == 0xFFFF) {
                V_PRINTF("NO_POT");
                set_error(INVALID_POT);
                return -1;
            }
            else if (pot_data_value > POT_MAX_VALUE) {
                pot_data_value = POT_MAX_VALUE;
            } else {
                pot_data_value = POT_MIN_VALUE;
            }

            break;
        }

    } while (pot_data_value > POT_MAX_VALUE || pot_data_value < POT_MIN_VALUE);

    *pot_data = pot_data_value;
    return 0;
}


/**
 * This function configures the ADC and confirms if config has been set
 *
 * return 0 if set and -1 otherwise
 */
int configHall(unsigned int config) {
    int returned_config = 0;
    unsigned char attempts = 0;

    do {

        if (attempts > 0)
        {
            //-- 10 ms
            __delay_cycles(10000);
        }

        P3OUT &= ~CS_HALL;
        //-- We receive the configuration value in the most significant 2 bytes
        spi_io(config, 2, CS_HALL);

        // Can only receive 2 bytes (2 byte registers on MSP430

        returned_config = spi_io(config, 2, CS_HALL);

        P3OUT |= CS_HALL;
        //-- bit shift 2 bytes to the right to get configuration value
        //returned_config >>= 16;

        if (++attempts > 4) {
            set_error(MAX_CONF_HALL_ATTEMPTS);
            return -1;
        }

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
static unsigned int spi_io(unsigned int data, int bytes, int chipSel) {
    int rx_buf, offset;
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



