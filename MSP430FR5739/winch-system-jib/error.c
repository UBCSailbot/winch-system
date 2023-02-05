/*
 * error.c
 *
 *  Created on: Feb 4, 2023
 *      Author: mlokh
 */
#include "error.h"


/**
 *  Name:       set_error
 *
 *
 *  Purpose:    capture the most recent error code
 *
 *  Params:     error_code_t - error code enum value
 *
 *  Return:     none
 *
 *  Notes:      increments error count. if error count > MAX_
 *              ERROR_COUNT then max_error_reached is set which
 *              would cause a software reset
 */
void set_error(error_code_t error)
{

}

/**
 *  Name:       get_error
 *
 *
 *  Purpose:    return the most recent error code previously captured
 *
 *  Params:     none
 *
 *  Return:     error_code_t - error code enum value
 *
 *  Notes:      error cleared after returned
 */
error_code_t get_error(void)
{

}

/**
 *  Name:       max_error_reached
 *
 *
 *  Purpose:    has the max error value reached
 *
 *  Params:     none
 *
 *  Return:     1 - max error count reached
 *              0 - max error count not reached
 *
 *  Notes:      flag is cleared after read
 */
unsigned char max_error_reached(void)
{

}
