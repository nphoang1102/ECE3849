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

// CPU load average interval in us
#define TIMER_CPU_LOAD_INT 10000

// Size of the period interval
#define PERIOD_INTERVAL 256

// Struct for global variable storage
struct TIMER {
    uint32_t cnt_last;
    uint32_t period[PERIOD_INTERVAL];
    uint8_t ptr;
    uint16_t period_accum;
    float frequency;
};
extern struct TIMER _timr;

/* Function prototypes */
void timer_oneshot_init(void); // initialize one shot timer for CPU load measurement
void timer_capture_init(void); // initialize timer capture mode and connection pins from external source
uint32_t timer_load_count(void); // counting for 10ms on call

#endif /* TIMER_H_ */
