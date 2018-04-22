/*
 * pwm_table.h
 *
 *  Created on: Apr 22, 2018
 *      Author: Hoang Nguyen
 *
 *  Separated module to store the 1024-sample waveform table
 */

#ifndef PWM_TABLE_H_
#define PWM_TABLE_H_

// Macro definition here
#define PWM_WAVEFORM_INDEX_BITS 10
#define PWM_WAVEFORM_TABLE_SIZE (1 << PWM_WAVEFORM_INDEX_BITS)

// Declare our table here and filled in under the .c file
extern uint8_t gPWMWaveformTable[PWM_WAVEFORM_TABLE_SIZE];

#endif /* PWM_TABLE_H_ */
