/*
 * uart.h
 *
 *  Created on: Apr. 21, 2022
 *      Author: mlokh
 */

#ifndef UART_H_
#define UART_H_


#define RXBUF_LEN 2

//-- State machine
#define PROCESS 0
#define READ 1
#define WAIT 2

//-- Init uart
void init_uart(void);

//-- Sends formated string to the UCCM
void uccm_send(const char *format, ...);

//-- Sends string to uart, to communicate with UCCM or debug
void putString(char* message);

//-- Indicates if command is ready or not
int isReady(void);

//-- Unclears the ready flag
void clearReady(void);

//-- Receive command from rxbuffer
char getMsg(void);




#endif /* UART_H_ */
