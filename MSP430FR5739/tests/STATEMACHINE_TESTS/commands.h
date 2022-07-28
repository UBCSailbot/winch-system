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
    unsigned int    active;
    unsigned int    type;
    t_state        state;
    unsigned int  rx_msg;       // FROM UCCM MSG
    unsigned int  tx_msg;       // TO UCCM MSG
}t_cmd;

static unsigned int active_cmd;
static unsigned int cmd_index = 0;
static unsigned int num_active_cmd = 0;
static t_cmd cmd_list[ACTIVE_CMD_SIZE];

static const t_cmd idle_cmd =
{
 1,
 IDLE_CMD,
 IDLE,
 0,
 0
};


//-- add a new premature command to the list - command has not yet decoded
void add_new_command(unsigned int rx_msg);

//-- Removes current command from the list
void end_command(void);

//-- Returns 1 if not busy and 0 otherwise
unsigned int is_busy(int cmd_id);

//-- Clears all other commands expect current
void clear_all_other_commands(void);


//-- GETERS and SETTERS --

//-- Sets the command type and/or tx_msg for the current command, and returns the next state
t_state set_current_command(unsigned int cmd_type, unsigned int tx_msg);

//-- Gets the command currently running
static t_cmd * get_current_command(void);

//-- Sets the current running state of the command
void set_current_command_state(t_state state);

//-- Gets the current running state of the command
t_state get_current_command_state(void);

//-- Set the values for the tx_msg variable in the t_cmd structure
void set_current_tx_msg(unsigned int tx_msg);

//-- Gets the value for the tx_msg variable in the t_cmd structure
unsigned int get_current_tx_msg(void);

//-- Sets the value of rx_msg in the t_cmd structure
void set_current_rx_msg(unsigned int rx_msg);

//-- Gets the value of rx_msg in the t_cmd structure
unsigned int get_current_rx_msg(void);


//-- HELPER FUNCTIONS --

//-- Increments to the next available command
static void find_next_active_cmd(void);

//-- Checks if the current command being pointed to by index is active
static unsigned int is_cmd_index_active(unsigned index);

//-- Looks up what the starting state of the command being ran is
static t_state lookup_cmd_start_state(unsigned int cmd_type);

//-- Returns 1 if available and 0 otherwise
static unsigned int is_command_available(void);

//-- Indicates if the max number of active commands have been reached
static unsigned int max_active_reached(void);

//-- Prints the command list
static void print_cmd_list(void);

#endif /* COMMANDS_H_ */
