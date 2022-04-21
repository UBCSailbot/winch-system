/*
 * spi.h
 *
 *  Created on: Apr. 3, 2022
 *      Author: mlokh
 */

#ifndef SPI_H_
#define SPI_H_

#define AIN0_CONF 0x14AA
#define AIN1_CONF 0x24AA
#define AIN2_CONF 0x34AA

#define CS_POT BIT6
#define CS_HALL BIT5

//-- Port J
#define NFAULT BIT3
#define MODE BIT2
#define PHASE BIT1
#define NSLEEP BIT0

//-- Port 1
#define ENABLE BIT4


//-- Initializes SPI peripheral
void init_spi(void);

//-- Receives data from hallsensors
void receive_hallsensors(int* pawl_left, int* cam, int* pawl_right);

//-- Configures the hallsensor by sending it the required config data
static int configHall(int config);

//-- Sends and receives integer data through SPI one byte at a time
static int spi_io(unsigned int data, int bytes);

#endif /* SPI_H_ */
