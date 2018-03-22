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

// Parameters for clock setup
#define ADC_SAMPLING_RATE 1000000   // [samples/sec] desired ADC sampling rate
#define CRYSTAL_FREQUENCY 25000000 

// Prameters for ISR
#define ADC_INT_PRIORITY 0

// initialize ADC1 for data sampling
void ADCinit(void);

// 

#endif /* ADC_H_ */
