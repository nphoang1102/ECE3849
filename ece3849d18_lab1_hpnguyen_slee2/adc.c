/*
 * adc.c
 *
 *  Created on: Mar 21, 2018
 *      Author: hpnguyen
 *
 *  ECE3849 Lab 1 ADC configuration for TM4C1294
 */


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "sysctl_pll.h"

// Initialize ADC1 for oscillator
void ADCinit(void) {

    // GPIO PE0 = Analog in 13
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); // GPIO setup pin E0


    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // initialize ADC peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    // ADC clock
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; //round up
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    ADCSequenceDisable(...); // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(...); // specify the "Always" trigger
    ADCSequenceStepConfigure(...);// in the 0th step, sample channel 3 (AIN3)
     // enable interrupt, and make it the end of sequence
    ADCSequenceEnable(...); // enable the sequence. it is now sampling
    ADCIntEnable(...); // enable sequence 0 interrupt in the ADC1 peripheral
    IntPrioritySet(...); // set ADC1 sequence 0 interrupt priority
    IntEnable(...); // enable ADC1 sequence 0 interrupt in int. controller
}
