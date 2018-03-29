/*
 * timer.h
 *
 *  Created on: Mar 29, 2018
 *      Author: hpnguyen
 *
 *  Header for timer.c, set up and configure miscellaneous timer
 *
 */

#ifndef TIMER_H_
#define TIMER_H_

/* Macro definition */

// CPU load average interval
#define TIMER_CPU_LOAD_INT 100

/* Function prototypes */
void timer_oneshot_init(void); // initialize one shot timer for CPU load measurement
uint32_t timer_load_count(void); // counting for 10ms on call

#endif /* TIMER_H_ */
