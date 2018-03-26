/*
 * adc.c
 *
 *  Created on: Mar 21, 2018
 *      Author: hpnguyen
 *
 *  ECE3849 Lab 1 ADC configuration for TM4C1294
 */

// Standard C libraries
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

// Driver libraries from TivaWare
#include "Crystalfontz128x128_ST7735.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "sysctl_pll.h"

// Libraries from project
#include "adc.h"
#include "buttons.h"
#include "lcd_display.h"

// Global variable declaration
volatile int32_t gADCBufferIndex = ADC_BUFFER_SIZE - 1; // latest sample index
volatile uint16_t gADCBuffer[ADC_BUFFER_SIZE];  // ring buffer
volatile uint32_t gADCErrors; // number of missed ADC deadlines
volatile uint16_t gScreenBuffer[FULL_SCREEN_SIZE] = {0};

// Initialize ADC1 for oscillator
void ADCinit(void) {

    // GPIO PE0 = Analog in 13
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); // GPIO setup pin E0

    // Enable ADC0 and ADC1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Setup the ADC clock
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; //round up
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);

    // Step configuration for ADC1
    ADCSequenceDisable(ADC1_BASE, 0); // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0); // specify the "Always" trigger
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);// in the 0th step, sample channel 3 (AIN3)
     // enable interrupt, and make it the end of sequence
    
    // Fire the interrupt and sequence now
    ADCSequenceEnable(ADC1_BASE, 0); // enable the sequence. it is now sampling
    ADCIntEnable(ADC1_BASE, 0); // enable sequence 0 interrupt in the ADC1 peripheral
    IntPrioritySet(INT_ADC1SS0, ADC_INT_PRIORITY); // set ADC1 sequence 0 interrupt priority
    IntEnable(INT_ADC1SS0); // enable ADC1 sequence 0 interrupt in int. controller
}


// ISR for ADC1 (gonna use only direct register access for faster processing time)
void ADC_ISR(void) {
    // First thing first, clear the interrupt flag so we can exit
    ADC1_ISC_R = ADC_ISC_IN0;
//    ADCIntClear(ADC1_BASE, 0);

    // Process the data coming in
    if (ADC1_OSTAT_R & ADC_OSTAT_OV0) { // check for ADC FIFO overflow
         gADCErrors++; // count errors
         ADC1_OSTAT_R = ADC_OSTAT_OV0; // clear overflow condition
     }
     gADCBuffer[
         gADCBufferIndex = ADC_BUFFER_WRAP(gADCBufferIndex + 1)
     ] = ADC1_SSFIFO0_R; // read sample from the ADC1 sequence 0 FIFO

}

// Searching for the trigger, rising = 1 for rising edge, rising = 0 for falling edge
uint32_t adc_trigger_search(uint16_t pTrigger, uint8_t rising) {

    // Choose a starting point for our buffer search
    int32_t start_index = ADC_BUFFER_WRAP(gADCBufferIndex - HALF_SCREEN_SIZE);
    int32_t search_index = start_index;

    // Start looking for the trigger value
    uint16_t search_iteration = 0;
    uint16_t drop_condition = ADC_BUFFER_SIZE >> 1;
    uint16_t current_value = gADCBuffer[search_index];
    switch(rising) {
    case 1: // case of rising edge
        while (current_value < pTrigger) {
            search_index = ADC_BUFFER_WRAP(search_index - 1);
            current_value = gADCBuffer[search_index];
            search_iteration++;

            // If we have been searching for a while, drop the operation return the initial search index
            if (search_iteration > drop_condition) return start_index;
        }
        break;

    case 0: // case of falling edge
        while (current_value > pTrigger) {
            search_index = ADC_BUFFER_WRAP(search_index - 1);
            current_value = gADCBuffer[search_index];
            search_iteration++;

            // If we have been searching for a while, drop the operation return the initial search index
            if (search_iteration > drop_condition) return start_index;
        }
        break;
    }

    // Return based on the result of the search
    return search_index;
}

// Copy samples half a screen behind and half a screen ahead of the trigger point
void adc_copy_buffer_samples(uint16_t pTrigger, uint8_t rising) {

    // Index range for buffer copy
    uint32_t half_behind = ADC_BUFFER_WRAP(adc_trigger_search(pTrigger, rising) - HALF_SCREEN_SIZE);
    uint16_t current_index = 0;
    int i;

    // Start copying over to our local buffer
    for (i = 0; i < FULL_SCREEN_SIZE; i++) {
        current_index = ADC_BUFFER_WRAP(half_behind + i);
        gScreenBuffer[i] = gADCBuffer[current_index];
    }
}

// Scaling the ADC sample in the vertical direction
uint16_t adc_y_scaling(float fVoltsPerDiv, uint16_t sample) {
    float fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);
    return LCD_VERTICAL_MAX/2 - (int)roundf(fScale * ((int)sample - ADC_OFFSET));
}
