/*
 * commands.h
 *
 *  Created on: Apr 25, 2022
 *      Author: Sailbot
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stddef.h>
#include "statemachine.h"

#define ACTIVE_CMD_SIZE 4

//-- command types
#define PREMATURE       0
#define SET_POS         1
#define QUERY_POS       2
#define STOPLOCK        4
#define ALIVE           8
#define UNDEF           16
#define ACTION_BUSY     32
#define IDLE_CMD        64


typedef struct cmd {
    unsigned int    free   = 0;
    unsigned int    type;
    enum State     state;
    unsigned int  rx_msg;       // FROM UCCM MSG
    unsigned int  tx_msg;       // TO UCCM MSG
}t_cmd;

static int active_cmd;
static int cmd_index = 0;
static unsigned int num_active_cmd = 0;
static t_cmd cmd_list[ACTIVE_CMD_SIZE];
static const t_cmd idle_cmd =
{
 IDLE_CMD,
 IDLE,
 0,
 0
};

//-- add a new premature command to the list - command has not yet decoded
void add_new_command(unsigned int rx_msg);

//-- Removes current command from the list
void end_command(void);

//-- Looks up what the starting state of the command being ran is
static enum States lookup_cmd_start_state(unsigned int cmd);

//-- Returns 1 if available and 0 otherwise
static unsigned int is_command_available(void);

//-- Returns 1 if not busy and 0 otherwise
unsigned int is_busy(int cmd_id);

//-- Indicates if the max number of active commands have been reached
unsigned int max_active_reached(void);

//-- Clears all active commands
void clear_active_commands(void);

//-- GETERS and SETTERS --

//-- Sets the command type and/or tx_msg for the current command, and returns the next state
enum States set_current_command(int cmd, unsigned int tx_msg);

//-- Saves the current state of the command in its structure
void save_current_state(unsigned int state);

//-- Set the values for the data variable in the command structure
void set_uccm_msg(unsigned int uccm_msg);

//-- Returns the command currently running
static t_cmd * get_current_command(void);

//-- Gets the current running state of the command - keeps incrementing  to the next command in the list
enum States get_current_command_state(void);

//-- HELPER FUNCTIONS --

static void inc_current_command_index(void);

static unsigned int is_cmd_index_free(unsigned index);

#endif /* COMMANDS_H_ */
