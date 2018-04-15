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
#include "driverlib/comp.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
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

// Initialize timer capture mode and connection pins from external source
void timer_capture_init(void) {

    /* Need to init the external source for our comparator first */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_COMP0);
    ComparatorRefSet(COMP_BASE, COMP_REF_1_65V);
    ComparatorConfigure(COMP_BASE, 1, COMP_INT_RISE);

    // configure GPIO for comparator input C1- at BoosterPack Connector #1 pin 3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // seems like pin C4 to me
    GPIOPinTypeComparator(GPIO_PORTC_BASE, GPIO_PIN_4);
 
    // configure GPIO for comparator output C1o at BoosterPack Connector #1 pin 15
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); // seems like pin D1 to me
    GPIOPinTypeComparatorOutput(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PD1_C1O); // search for PART_TM4C1294NCPDT under driverlib/pin_map.h

    // configure GPIO PD0 as timer input T0CCP0 at BoosterPack Connector #1 pin 14
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinConfigure(GPIO_PD0_T0CCP0);

    /* Now init the capture mode */
    // Now configure the timer as capture mode
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP);
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 0xffff); // use maximum load value
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0xff); // use maximum prescale value
    
    /* Need to verify this actually */
    TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT);
    
    TimerEnable(TIMER0_BASE, TIMER_A);
}

// Timer capture mode ISR
void timer_capture_ISR(void) {
    // Code to be implemented later
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
