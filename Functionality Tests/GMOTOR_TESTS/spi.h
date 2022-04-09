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

#define CS_POT BIT6
#define CS_HALL BIT5


//-- Initializes SPI peripheral
void init_spi(void);

//-- Receives data from hallsensors
void receive_hallsensors(int* pawl_left, int* cam, int* pawl_right);

//-- Receives data from potentiometer
void receive_potentiometer(unsigned int* pot_data);

//-- Configures the hallsensor by sending it the required config data
int configHall(unsigned int config);

//-- Sends and receives integer data through SPI one byte at a time
static unsigned long spi_io(unsigned int data, int bytes, int chipSel);


#endif /* SPI_H_ */
