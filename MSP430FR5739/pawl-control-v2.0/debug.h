/*
 * debug.h
 *
 *  Created on: Mar. 28, 2022
 *      Author: mlokh
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#define DBG 1
#define V_PRINTF(format, ...) while(DBG) {dbg_printf(format, ##__VA_ARGS__); break;}

// used to send messages over UART for debugging purposes
void dbg_printf(const char *format, ...);

void init_UART_DBG(void);
void putString(char* message);


#endif /* DEBUG_H_ */
