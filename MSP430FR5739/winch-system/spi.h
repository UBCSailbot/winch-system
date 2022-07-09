/*
 * spi.h
 *
 *  Created on: Apr. 3, 2022
 *      Author: mlokh
 */

#ifndef SPI_H_
#define SPI_H_

#define AIN0_CONF 0x14AB
#define AIN1_CONF 0x24AB
#define AIN2_CONF 0x34AB

#define AIN0_CONFID         1
#define AIN1_CONFID         2
#define AIN2_CONFID         4

#define CS_POT BIT6
#define CS_HALL BIT5

#define MAX_POT_TRIES       3
#define POT_MAX_VALUE    4095

//-- Tracks current configs
static unsigned int active_config;

//-- Initializes SPI peripheral
void init_spi(void);

//-- Receives data from hallsensors
int receive_hallsensors(unsigned int* pawl_left, int* cam,unsigned int* pawl_right);

//-- Receives data from potentiometer. Return -1 if voltage not in the range
int receive_potentiometer(unsigned int* pot_data);

//-- Configures the hallsensor by sending it the required config data
int configHall(unsigned int config);

//-- Sends and receives integer data through SPI one byte at a time
static unsigned int spi_io(unsigned int data, int bytes, int chipSel);


#endif /* SPI_H_ */
