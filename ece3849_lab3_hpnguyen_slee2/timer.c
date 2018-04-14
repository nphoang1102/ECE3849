/*
 * timer.c
 *
 *  Created on: Mar 29, 2018
 *      Author: hpnguyen
 */

// Standard C libraries
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// TivaWare driver libraries
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "sysctl_pll.h"

// Libraries from project
#include "timer.h"

// Importing global variable
extern uint32_t gSystemClock;

// Initialize one shot timer for calculating CPU load
void timer_oneshot_init(void) {

    // Using timer 3 as our oneshot timer
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    TimerDisable(TIMER3_BASE, TIMER_BOTH);
    TimerConfigure(TIMER3_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER3_BASE, TIMER_A, TIMER_CPU_LOAD_INT - 1);
}

// Count up for 10ms time period and return counted value
uint32_t timer_load_count(void) {
    uint32_t i = 0;
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER3_BASE, TIMER_A);
    while (!(TimerIntStatus(TIMER3_BASE, false) & TIMER_TIMA_TIMEOUT))
        i++;
    return i;
}
