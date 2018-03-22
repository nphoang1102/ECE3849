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
#include "adc.h"

// Initialize ADC1 for oscillator
void ADCinit(void) {

    // GPIO PE0 = Analog in 13
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); // GPIO setup pin E0

    // Clock configuration for ADC1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; //round up
    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);

    // Initialize ADC1 peripheral
    ADCSequenceDisable(ADC1_BASE, 0); // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0); // specify the "Always" trigger
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);// in the 0th step, sample channel 3 (AIN3)
     // enable interrupt, and make it the end of sequence
    
    // Hook up the ISR
    IntPrioritySet(INT_ADC1SS0, 29); // set ADC1 sequence 0 interrupt priority
    ADCIntEnable(ADC1_BASE, 0); // enable sequence 0 interrupt in the ADC1 peripheral
    
    // Fire the interrupt and sequence now
    IntEnable(INT_ADC1SS0); // enable ADC1 sequence 0 interrupt in int. controller
    ADCSequenceEnable(ADC1_BASE, 0); // enable the sequence. it is now sampling
}


// ISR for ADC1
void ADC_ISR(void) {
    // First thing first, clear the interrupt flag so we can exit


    if (ADC1_OSTAT_R & ADC_OSTAT_OV0) { // check for ADC FIFO overflow
         gADCErrors++; // count errors
         ADC1_OSTAT_R = ADC_OSTAT_OV0; // clear overflow condition
     }
     gADCBuffer[
         gADCBufferIndex = ADC_BUFFER_WRAP(gADCBufferIndex + 1)
     ] = <...>; // read sample from the ADC1 sequence 0 FIFO

}