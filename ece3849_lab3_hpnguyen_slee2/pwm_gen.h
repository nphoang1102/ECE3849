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
#define PWM_WAVEFORM_INDEX_BITS 10
#define PWM_WAVEFORM_TABLE_SIZE (1 << PWM_WAVEFORM_INDEX_BITS)

// Global struct for data storage
struct PWM {
    uint32_t gPhase;              // phase accumulator
    uint32_t gPhaseIncrement;     // phase increment for 20 kHz
    uint8_t gPWMWaveformTable[PWM_WAVEFORM_TABLE_SIZE];
};
extern struct PWM _pwm;

uint32_t gPhase = 0;              // phase accumulator
uint32_t gPhaseIncrement = ?;     // phase increment for 20 kHz


uint8_t gPWMWaveformTable[PWM_WAVEFORM_TABLE_SIZE] = {0};

// Function prototypes
void pwm_init(void); // Initialize the PWM generator pins

#endif /* PWM_GEN_H_ */
