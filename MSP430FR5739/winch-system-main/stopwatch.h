/*
 * stopwatch.h
 *
 *  Created on: Apr. 10, 2023
 *      Author: mlokh
 */

#ifndef STOPWATCH_H_
#define STOPWATCH_H_

static unsigned int timer_cnt;
static unsigned char stopwatch_en;
static unsigned int prev_capture;

void init_capture_timer(void);
void start_stopwatch(void);
unsigned int cap_diff_stopwatch(void);
unsigned int stop_stopwatch(void);
unsigned char stopwatch_enabled(void);

void init_capture_timer(void)
{

//    //-- ENABLE p1.1
//    P1DIR &= ~BIT1;
//    P1IN |= BIT1;

    //-- SMCLK
    TA0CTL |= TBSSEL_2;

    //-- TA0CCTL0 reg used for capture on failing edge
    TA0CCTL0 |= CM1;

    TA0CCTL0 |= CAP;

    //-- Trigger from GPIO pin P1.1
    TA0CCTL2 |= CCIS_0;

    stopwatch_en = 0;
    prev_capture = 0;
}

void start_stopwatch(void)
{
    //-- Enable Interrupt
    TA0CTL |= TAIE;
    //-- MC - Continuous
    TA0CTL |= MC__CONTINUOUS;

    stopwatch_en = 1;
    prev_capture = 0;
}

unsigned int cap_diff_stopwatch(void)
{
    unsigned int time = TA0R;
    unsigned int diff;

    diff = prev_capture - time;
    prev_capture = time;

    return diff;
}

unsigned int stop_stopwatch(void)
{
    unsigned int time;

    TA0CTL &= ~TAIE;
    //-- MC - Continuous
    TA0CTL &= ~MC__CONTINUOUS;

    time = TA0R;
    TA0CTL |= TACLR;

    stopwatch_en = 0;

    return time;
}

unsigned char stopwatch_enabled(void)
{
    return stopwatch_en;
}


#endif /* STOPWATCH_H_ */
