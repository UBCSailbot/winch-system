/*
 * debug.c
 *
 *  Created on: Mar. 28, 2022
 *      Author: mlokh
 */
#include <msp430.h>
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"
#include "uart.h"


void dbg_printf(const char *format, ...) {
    va_list args;
    char str[100] = "";

    va_start(args, format);

    vsprintf(str, format, args);

    putString(str);

    va_end(args);
}





