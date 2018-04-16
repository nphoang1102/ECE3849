/*
 * main.c
 * Gene Bogdanov - 3/18/2013, modified 11/3/2017
 * ECE 3849 Real-time Embedded Systems
 * Code for measuring ISR latency, response time and
 * determining if deadlines are met.  Runs under TI-RTOS.
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#define delay_us(us) SysCtlDelay((us) * 40) // delay in us assuming 120 MHz operation and 3-cycle loop

// event and handler definitions
#define EVENT0_PERIOD           6007    // [us] event0 period
#define EVENT0_EXECUTION_TIME   2000    // [us] event0 handler execution time

#define EVENT1_PERIOD           8101    // [us] event1 period
#define EVENT1_EXECUTION_TIME   1000    // [us] event1 handler execution time

#define EVENT2_PERIOD           12301   // [us] event2 period
#define EVENT2_EXECUTION_TIME   6000    // [us] event2 handler execution time

// measured event latencies in clock cycles
unsigned long event0_latency = 0;
unsigned long event1_latency = 0;
unsigned long event2_latency = 0;

// measured event response time in clock cycles
unsigned long event0_response_time = 0;
unsigned long event1_response_time = 0;
unsigned long event2_response_time = 0;

// number of deadlines missed
unsigned long event0_missed_deadlines = 0;
unsigned long event1_missed_deadlines = 0;
unsigned long event2_missed_deadlines = 0;

// timer periods in clock cycles (expecting 120 MHz clock)
#define TIMER1_PERIOD (120 * EVENT0_PERIOD)
#define TIMER2_PERIOD (120 * EVENT1_PERIOD)
#define TIMER3_PERIOD (120 * EVENT2_PERIOD)

// CPU load counters
unsigned long count_unloaded = 0;
unsigned long count_loaded = 0;
float cpu_load = 0.0;

// function prototypes
void event0_handler(void);
void event1_handler(void);
void event2_handler(void);

void main(void) {
    IntMasterDisable();

    // initialize general purpose timers 1-3 for periodic interrupts
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerDisable(TIMER1_BASE, TIMER_BOTH);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, TIMER1_PERIOD - 1);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    TimerDisable(TIMER2_BASE, TIMER_BOTH);
    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER2_BASE, TIMER_A, TIMER2_PERIOD - 1);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    TimerDisable(TIMER3_BASE, TIMER_BOTH);
    TimerConfigure(TIMER3_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER3_BASE, TIMER_A, TIMER3_PERIOD - 1);
    TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);

    BIOS_start();     /* start SYS/BIOS */
}

void event0_handler(void)
{
    unsigned long t = TIMER1_PERIOD - TIMER1_TAR_R;
    if (t > event0_latency) event0_latency = t; // measure latency
    TIMER1_ICR_R = TIMER_ICR_TATOCINT; // clear interrupt flag

    delay_us(EVENT0_EXECUTION_TIME); // handle event0

    if (TIMER1_MIS_R & TIMER_MIS_TATOMIS) { // next event occurred
        event0_missed_deadlines++;
        t = 2 * TIMER1_PERIOD; // timer overflowed since last event
    }
    else t = TIMER1_PERIOD;
    t -= TIMER1_TAR_R;
    if (t > event0_response_time) event0_response_time = t; // measure response time
}

void event1_handler(void)
{
    unsigned long t = TIMER2_PERIOD - TIMER2_TAR_R;
    if (t > event1_latency) event1_latency = t;
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;

    delay_us(EVENT1_EXECUTION_TIME); // handle event1

    if (TIMER2_MIS_R & TIMER_MIS_TATOMIS) { // next event occurred
        event1_missed_deadlines++;
        t = 2 * TIMER2_PERIOD; // timer overflowed since last event
    }
    else t = TIMER2_PERIOD;
    t -= TIMER2_TAR_R;
    if (t > event1_response_time) event1_response_time = t;
}

void event2_handler(void)
{
    unsigned long t = TIMER3_PERIOD - TIMER3_TAR_R;
    if (t > event2_latency) event2_latency = t;
    TIMER3_ICR_R = TIMER_ICR_TATOCINT;

    delay_us(EVENT2_EXECUTION_TIME); // handle event2

    if (TIMER3_MIS_R & TIMER_MIS_TATOMIS) { // next event occurred
        event2_missed_deadlines++;
        t = 2 * TIMER3_PERIOD; // timer overflowed since last event
    }
    else t = TIMER3_PERIOD;
    t -= TIMER3_TAR_R;
    if (t > event2_response_time) event2_response_time = t;
}

void task0_func(UArg arg0, UArg arg1)
{
    IntMasterEnable();

    TimerEnable(TIMER1_BASE, TIMER_A);
    TimerEnable(TIMER2_BASE, TIMER_A);
    TimerEnable(TIMER3_BASE, TIMER_A);

    while (1) {
        Semaphore_post(task1_sem);
        Semaphore_pend(task0_sem, BIOS_WAIT_FOREVER);
        delay_us(200);
    }
}

void task1_func(UArg arg0, UArg arg1)
{
    while (1) {
        Semaphore_post(task0_sem);
        Semaphore_pend(task1_sem, BIOS_WAIT_FOREVER);
        delay_us(200);
    }
}
