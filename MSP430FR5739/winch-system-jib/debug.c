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


/**
 *  Name:       dbg_printf
 *
 *
 *  Purpose:    outputs a formatted string to UART for debug
 *
 *  Params:     format - formated string,
 *              multiple arguments
 *
 *  Return:     none
 *
 *  Notes:      debug using V_PRINTF macro only. Ensure that DBG is set to 1.
 */
void dbg_printf(const char *format, ...) {
    va_list args;
    char str[100] = "";

    va_start(args, format);

    vsprintf(str, format, args);

    putString(str);

    va_end(args);
}





