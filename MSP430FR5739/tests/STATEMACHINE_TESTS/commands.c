/*
 * commands.c
 *
 *  Created on: Jul. 16, 2022
 *      Author: mlokh
 */
#include "commands.h"
#include "debug.h"


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

        for (i = 0; i < ACTIVE_CMD_SIZE; i++) {
            if ( !is_cmd_index_active(i)) {
                V_PRINTF("ADD cmd to (%d)", i)
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

        new_cmd->active = 1;

        num_active_cmd++;

        print_cmd_list();
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
        cur_cmd->active = 0;

        print_cmd_list();
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
    current_cmd = get_current_command();

    if (current_cmd == (t_cmd*)0) {
        //V_PRINTF("-> GOTO IDLE \r\n")
        //-- No current command available
        return IDLE;
    }

    if (is_busy(cmd_type)) {
        V_PRINTF(" BUSY ->")
        current_cmd->type = ACTION_BUSY;
        current_cmd->state = SEND_TO_UCCM;
        current_cmd->tx_msg = BUSY_MSG << 9;
    } else {
        current_cmd->type = cmd_type;
        current_cmd->state = lookup_cmd_start_state(cmd_type);
        current_cmd->tx_msg = tx_msg;
        active_cmd |= cmd_type;
    }

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
 *              nullptr - if current command is not active or cmd_index out of bounds
 *
 *  Notes:      check if t_cmd returned is not a nullptr before using it
 */
static t_cmd * get_current_command(void) {
    return get_command(cmd_index);
}

/**
 *  Name:       get_command
 *
 *
 *  Purpose:    gets the command at a certain index if that command is active
 *
 *  Params:     index - the index the command is located
 *
 *  Return:     pointer to the active command
 *              nullptr - if current command is not active or cmd_index out of bounds
 *
 *  Notes:      check if t_cmd returned is not a nullptr before using it
 */
static t_cmd * get_command(unsigned int index) {
    t_cmd * current_cmd;

    if (index >= ACTIVE_CMD_SIZE) {
        return (t_cmd*)0;
    }

    current_cmd = &cmd_list[index];

    if (!current_cmd->active) {
        return (t_cmd*)0;
    }

    return current_cmd;
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
 *  Notes:      NOP if current command is not active and if setting to IDLE state
 */
void set_current_command_state(t_state state) {
    t_cmd * cur_cmd = get_current_command();

    if (cur_cmd != (t_cmd*)0 && state != IDLE) {
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
        //V_PRINTF("-> GOTO IDLE \r\n")
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
        start_state = SET_DIRECTION;
        break;

    case QUERY_POS:
        start_state = GET_POSITION;
        break;

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
    unsigned int i;

    for (i = 0; i < ACTIVE_CMD_SIZE; i++) {
        current_cmd = get_command(i);

        if (current_cmd != (t_cmd *)0 && i != cmd_index) {

            num_active_cmd -= current_cmd->active;

            current_cmd->active = 0;
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
 *  Notes:      ensure that the current command is active before calling
 *              this function. If no other command is found then the
 *              current running command remains the same
 */
static void find_next_active_cmd(void) {
    int count = 0;
    unsigned int index = cmd_index;

    //-- Only if there are active commands
    if (is_command_available()) {
        do {
            index = (++index) % ACTIVE_CMD_SIZE;
            count++;
        } while ( count < ACTIVE_CMD_SIZE && !is_cmd_index_active(index) );

    }

    cmd_index = index;
}

/**
 *  Name:       is_cmd_index_active
 *
 *
 *  Purpose:    checks if the command in the list at position index is active
 *
 *  Params:     none
 *
 *  Return:     1 - index points to a command that is acitve
 *              0 - index points to a command that is not acitve
 *
 *  Notes:      ensure that index < ACTIVE_CMD_SIZE
 */
static unsigned int is_cmd_index_active(unsigned index) {
    t_cmd * command;

    command = get_command(index);

    if (command != (t_cmd *)0) {
        return command->active;
    }
    else return 0;
}

/**
 *  Name:       print_cmd_list
 *
 *
 *  Purpose:    for debugging to see the commands in the list
 *
 *  Params:     none
 *
 *  Return:     none
 *
 *  Notes:      make sure debug is enabled in debug.h
 */
void print_cmd_list(void) {
    int i;

    V_PRINTF("\n\r")

    for (i = 0; i < ACTIVE_CMD_SIZE; i++) {
        V_PRINTF("|  (%d) ", is_cmd_index_active(i))
    }

    V_PRINTF("| num_active: %d active_cmds: %x\r\n", num_active_cmd, active_cmd)
}


