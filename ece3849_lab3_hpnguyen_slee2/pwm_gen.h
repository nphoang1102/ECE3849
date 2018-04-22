/*
 * pwm_gen.h
 *
 *  Created on: Apr 22, 2018
 *      Author: hpnguyen
 *
 *  Header file for PWM function generator
 */

#ifndef PWM_GEN_H_
#define PWM_GEN_H_

// Macro definition
#define PWM_PERIOD 258  // PWM period = 2^8 + 2 system clock cycles

// Function prototypes
void pwm_init(void); // Initialize the PWM generator pins

#endif /* PWM_GEN_H_ */
