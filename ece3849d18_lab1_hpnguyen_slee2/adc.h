/*
 * adc.h
 *
 *  Created on: Mar 21, 2018
 *      Author: hpnguyen
 *
 *  Header for adc.c, aiming for 1Msps sampling rate
 *
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

/* Macro definition */

// ADC declaration
#define ADC_SAMPLING_RATE 1000000   // [samples/sec] desired ADC sampling rate +// Timer interrupt macro
#define CRYSTAL_FREQUENCY 25000000

// Definition for interrupt setup
#define ADC_INT_PRIORITY 0 // highest priority considering 1us time period

// Definition for ring buffer size and wrapping helper
#define ADC_BUFFER_SIZE 2048 // 2kbit buffer size
#define ADC_BUFFER_WRAP(i) ((i) & (ADC_BUFFER_SIZE - 1)) // index wrapping

// Scaling definition
#define ADC_OFFSET 2048
#define VIN_RANGE 3.3f
#define ADC_BITS 12

/* Function prototypes */
void ADCinit(void); // initialize ADC1 for data sampling
uint32_t adc_trigger_search(uint16_t pTrigger, uint8_t rising); // searching for the trigger index position
void adc_copy_buffer_samples(uint16_t pTrigger, uint8_t rising); // copy a screen worth of samples to a buffer
uint16_t adc_y_scaling(float fVoltsPerDiv, uint16_t sample); // scaling the ADC sample in the vertical direction

#endif /* ADC_H_ */
