/*
 * commands.c
 *
 *  Created on: Jul. 16, 2022
 *      Author: mlokh
 */
#include "commands.h"


/**
 *  Name:       add_new_command
 *
 *
 *  Purpose:    inserts a new command to the list that has not been decoded yet
 *
 *  Params:     rx_msg - message received from the uccm for this command
 *
 *  Return:     none
 *
 *  Notes:      none
 */
void add_new_command(unsigned int rx_msg) {
    t_cmd * new_cmd = (t_cmd *)(0);
    int i;

    if ( !max_active_reached() ) {

        for (i = 0; i <= ACTIVE_CMD_SIZE; i++) {
            if ( is_cmd_index_free(i) ) {
                new_cmd = &cmd_list[i];
                break;
            }
        }

        if (new_cmd == (t_cmd *)(0)) {
            //-- no free space found for command
            return;
        }

        new_cmd->type = PREMATURE;
        new_cmd->state = DECODE;
        new_cmd->rx_msg = rx_msg;
        new_cmd->tx_msg = 0;

        //-- Not free anymore (cannot be used by another command)
        new_cmd->free = 0;

        num_active_cmd++;
    }
}

/**
 *  Name:       end_command
 *
 *
 *  Purpose:    removes current active command from the list. NOP if no active commands
 *
 *  Params:     none
 *
 *  Return:     none
 *
 *  Notes:      does not increment the cmd_index to the next active cmd ie the current command
 *              would be pointing to a freed cmd which is not valid
 */
void end_command(void) {
    t_cmd * cur_cmd = get_current_command();
    if (cur_cmd != (t_cmd*)0) {

        //-- Clear current command from active
        active_cmd &= ~cur_cmd->type;
        num_active_cmd--;

        //-- Space is free to use by any other command
        cur_cmd->free = 1;

    }
}

/**
 *  Name:       set_current_command
 *
 *
 *  Purpose:    sets the current command's type
 *              - PREMATURE
 *              - SET_POS
 *              - QUERY_POS
 *              - STOPLOCK
 *              - ALIVE
 *              - UNDEF
 *              - ACTION_BUSY
 *              - IDLE_CMD
 *
 *  Params:     cmd_type - the type of command to set
 *              tx_msg   - the message which would be sent to the UCCM
 *                         once command is successful
 *
 *  Return:     the next state the command would be set to
 *              IDLE if current command does not exist
 *
 *  Notes:      none
 */
t_state set_current_command(unsigned int cmd_type, unsigned int tx_msg) {
    t_cmd * current_cmd;

    if (is_busy(cmd_type)) {
        cmd_type = ACTION_BUSY;
        tx_msg = BUSY_MSG << 9;
    }

    current_cmd = get_current_command();

    if (current_cmd == (t_cmd*)0) {
        //-- No current command available
        return IDLE;
    }

    current_cmd->type = cmd_type;
    current_cmd->state = lookup_cmd_start_state(cmd_type);
    current_cmd->tx_msg = tx_msg;

    return current_cmd->state;
}

/**
 *  Name:       get_current_command
 *
 *
 *  Purpose:    gets the current active command
 *
 *  Params:     none
 *
 *  Return:     pointer to the current active command
 *              nullptr - if current command is free or cmd_index out of bounds
 *
 *  Notes:      check if t_cmd is not a nullptr before using it
 */
static t_cmd * get_current_command(void) {

    if (cmd_index >= ACTIVE_CMD_SIZE) {
        return (t_cmd*)0;
    }

    t_cmd * current_cmd = &cmd_list[cmd_index];

    if (current_cmd->free) {
        return (t_cmd*)0;
    }

    return &cmd_list[cmd_index];
}

/**
 *  Name:       set_current_command_state
 *
 *
 *  Purpose:    sets the current state the command is in
 *
 *  Params:     state - the current state to be set
 *
 *  Return:     none
 *
 *  Notes:      NOP if current command is free or
 *              not available
 */
void set_current_command_state(t_state state) {
    t_cmd * cur_cmd = get_current_command();

    if (cur_cmd != (t_cmd*)0) {
        cur_cmd->state = state;
    }
}

/**
 *  Name:       get_current_command_state
 *
 *
 *  Purpose:    gets the current state of the active command
 *
 *  Params:     none
 *
 *  Return:     state of the active command
 *              IDLE if no more command active
 *
 *  Notes:      this function processes a new active command
 *              in the list every time it is called
 */
t_state get_current_command_state(void) {
    t_cmd * current_cmd;
    t_state current_state;

    //-- Want to keep processing the next command each time we get the state value
    find_next_active_cmd();

    current_cmd = get_current_command();

    if (current_cmd == (t_cmd *)0) {
        current_state = IDLE;
    } else {
        current_state = current_cmd->state;
    }


    return current_state;
}

/**
 *  Name:       lookup_cmd_start_state
 *
 *
 *  Purpose:    lookup to find the first next_state the command type would run
 *
 *  Params:     cmd_type - the type of the command to lookup
 *
 *  Return:     the first next state for each command
 *
 *  Notes:      idle command types will have an abort state as
 *              this type is not supposed to be set
 */
static t_state lookup_cmd_start_state(unsigned int cmd_type) {
    t_state start_state = IDLE;

    switch(cmd_type) {

    case SET_POS:
        start_state = TURN_MOTOR_ON;
        break;

    case QUERY_POS:
    case ALIVE:
    case UNDEF:
    case ACTION_BUSY:
        start_state = SEND_TO_UCCM;
        break;

    case IDLE_CMD:
        // Never set a new idle command

    case STOPLOCK:
    default:
        start_state = ABORT;
        break;

    }

    return start_state;
}

/**
 *  Name:       set_current_tx_msg
 *
 *
 *  Purpose:    sets the message to be transmitted to the UCCM
 *
 *  Params:     tx_msg - message to be sent to UCCM
 *
 *  Return:     none
 *
 *  Notes:      NOP if no current command is not available
 */
void set_current_tx_msg(unsigned int tx_msg) {
    t_cmd * current_cmd = get_current_command();
    if (current_cmd != (t_cmd*)0) {
        current_cmd->tx_msg = tx_msg;
    }
}

/**
 *  Name:       get_current_tx_msg
 *
 *
 *  Purpose:    gets the message to be transmitted to the UCCM
 *
 *  Params:     none
 *
 *  Return:     0 when no active command
 *
 *  Notes:      none
 */
unsigned int get_current_tx_msg(void) {
    t_cmd * current_cmd = get_current_command();
    unsigned int current_tx_msg;

    if (current_cmd == (t_cmd *)0) {
        current_tx_msg = 0;
    } else {
        current_tx_msg = current_cmd->tx_msg;
    }

    return current_tx_msg;
}

/**
 *  Name:       set_current_rx_msg
 *
 *
 *  Purpose:    sets the rx msg that was received from the UCCM
 *              for the current active command
 *
 *  Params:     rx_msg - message received from the uccm
 *
 *  Return:     none
 *
 *  Notes:      none
 */
void set_current_rx_msg(unsigned int rx_msg) {
    t_cmd * current_cmd = get_current_command();

    if (current_cmd != (t_cmd *)0) {
        current_cmd->rx_msg = rx_msg;
    }
}

/**
 *  Name:       get_current_rx_msg
 *
 *
 *  Purpose:    gets the rx msg that was received from the UCCM
 *              for the current active command
 *
 *  Params:     none
 *
 *  Return:     rx msg received from the UCCM for the current
 *              active command
 *              if current command is not active then return 0
 *
 *  Notes:      none
 */
unsigned int get_current_rx_msg(void) {
    t_cmd * current_cmd;
    unsigned int current_rx_msg;

    current_cmd = get_current_command();

    if (current_cmd == (t_cmd *)0) {
        current_rx_msg = 0;
    } else {
        current_rx_msg = current_cmd->rx_msg;
    }

    return current_rx_msg;
}

/**
 *  Name:       is_command_available
 *
 *
 *  Purpose:    checks if there are any available active commands
 *
 *  Params:     none
 *
 *  Return:     1 - active command available
 *              0 - no active commands
 *
 *  Notes:      none
 */
static unsigned int is_command_available(void) {
    return !(num_active_cmd <= 0);
}

/**
 *  Name:       is_busy
 *
 *
 *  Purpose:    if the cmd_id matches a command that is currently running
 *
 *  Params:     cmd_id - the id of the command to check against active_cmd
 *
 *  Return:     none
 *
 *  Notes:      none
 */
unsigned int is_busy(int cmd_id) {
    return active_cmd & cmd_id;
}

/**
 *  Name:       max_active_reached
 *
 *
 *  Purpose:    checks if there is any space in the list for another command
 *
 *  Params:     none
 *
 *  Return:     1 - max space capacity of ACTIVE_CMD_SIZE reached
 *              0 - available space in the list
 *
 *  Notes:      none
 */
static unsigned int max_active_reached(void) {
    return num_active_cmd >= ACTIVE_CMD_SIZE;
}

/**
 *  Name:       clear_all_other_commands
 *
 *
 *  Purpose:    removes all commands except the current active one
 *
 *  Params:     none
 *
 *  Return:     none
 *
 *  Notes:      none
 */
void clear_all_other_commands(void) {
    t_cmd * current_cmd = (t_cmd *)(0);
    int i;

    for (i = 0; i <= ACTIVE_CMD_SIZE; i++) {
        current_cmd = &cmd_list[i];

        if (i != cmd_index) {
            current_cmd->free = 1;
            active_cmd &= ~current_cmd->type;
        }
    }
}

/**
 *  Name:       find_next_active_cmd
 *
 *
 *  Purpose:    increments cmd_index to the next command that is active
 *
 *  Params:     none
 *
 *  Return:     none
 *
 *  Notes:      ensure that the current command is not free before calling
 *              this function. If no other command is found then the
 *              current running command remains the same
 */
static void find_next_active_cmd(void) {
    int count = 0;
    unsigned int index = cmd_index;

    if (num_active_cmd == 1) {
        // If only the current command is active - NOP
       return;
    }

    //-- Only if there are active commands
    if (is_command_available()) {
        do {
            index = (++index) % ACTIVE_CMD_SIZE;
            count++;
        } while ( count < ACTIVE_CMD_SIZE - 1 && is_cmd_index_free(index) );


        if (count == ACTIVE_CMD_SIZE - 1) {
            //-- if we have gone through all possible commands leave cmd_index to be the same
            index = cmd_index;
        }

    }

    cmd_index = index;
}

/**
 *  Name:       is_cmd_index_free
 *
 *
 *  Purpose:    checks if the command in the list at position index is free
 *
 *  Params:     none
 *
 *  Return:     1 - index points to command free to use
 *              0 - index points to current running command that is not freed
 *
 *  Notes:      ensure that index < ACTIVE_CMD_SIZE
 */
static unsigned int is_cmd_index_free(unsigned index) {
    if (index < ACTIVE_CMD_SIZE) {
        return cmd_list[index].free;
    }
    else return 1;
}




