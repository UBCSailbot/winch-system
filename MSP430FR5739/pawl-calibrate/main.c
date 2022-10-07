#include <msp430.h>
#include <stdio.h>
#include "uart.h"
#include "spi.h"
#include "debug.h"
#include "gearmotor.h"

#define FORWARD     1
#define BACKWARD    0

#define MAX_SPEED_NUM   3

const int speeds[MAX_SPEED_NUM] =
{
//              even_more_slow,
              SUPER_SLOW,
              SLOW,
              MEDIUM,
//              FAST
};

typedef struct control_tracker {
    unsigned int speed_sel;
    int forward;
}t_control_tracker;


void init(void);

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

	init();
	V_PRINTF("MAIN\r\n");

	char msg;
    t_control_tracker tracker;

	unsigned int pawl_right;
	unsigned int pawl_left;
	int cam;

	while (1)
	{
	    if (isReady())
	    {
	        msg = getMsg();
	        V_PRINTF("msg: %c\r\n", msg);

	        switch(msg)
	        {

	        case 'a':       //-- Forward towards right pawl
	            startGearMotor(FORWARD, speeds[tracker.speed_sel], NO_TIMEOUT);
	            tracker.forward = 1;
	            break;

	        case 'd':       //-- Backward towards left pawl
                startGearMotor(BACKWARD, speeds[tracker.speed_sel], NO_TIMEOUT);
                tracker.forward = 0;
                break;

	        case 's':       //-- STOP
	            stopGearMotor();
	            startGearMotor(~tracker.forward, speeds[tracker.speed_sel], 50);
	            break;

	        case 'e':      //-- increase speed_sel
	            if (++tracker.speed_sel >= MAX_SPEED_NUM)
                 {
                    tracker.speed_sel = MAX_SPEED_NUM -1;
                 }
	            break;

	        case 'q':      //-- decrease speed_sel
                if (tracker.speed_sel <= 0) {
                    tracker.speed_sel = 0;
                } else {
                    tracker.speed_sel--;
                }
                break;

	        case '1':      //-- print right pawl spi value
	            receive_hallsensors(NULL, NULL, &pawl_right);
	            V_PRINTF("RIGHT_PAWL: %x\r\n",pawl_right);
	            break;

	        case '2':       //-- print CAM spi value
                receive_hallsensors(NULL, &cam, NULL);
                V_PRINTF("CAM: %d\r\n",cam);
	            break;

	        case '3':       //-- print left pawl spi value
                receive_hallsensors(&pawl_left, NULL, NULL);
                V_PRINTF("LEFT_PAWL: %x\r\n",pawl_left);
	            break;

	        default:
	            break;

	        }

	        clearReady();
	    }
	}

	return 0;
}

void init(void) {
    init_gearmotor();
    init_spi();
    init_uart();

    __enable_interrupt();
}
