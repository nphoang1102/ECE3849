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
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/udma.h"
#include "sysctl_pll.h"

// Libraries from project
#include "adc.h"
#include "buttons.h"
#include "kiss_fft.h"
#include "_kiss_fft_guts.h"
#include "lcd_display.h"
#include "RTOS_helper.h"

// Pragma to allocate DMA control table in RAM and global variable stuffs for DMA
#pragma DATA_ALIGN(gDMAControlTable, 1024) // address alignment required
tDMAControlTable gDMAControlTable[64]; // uDMA control table (global)
volatile bool gDMAPrimary = true; // is DMA occurring in the primary channel?

// Initialize struct space for global variable
struct ADC _adc = {
    {0}, // ring buffer
    0, // number of missed ADC deadlines
};

// Importing global variable
extern uint32_t gSystemClock;
extern const float gVoltageScale[];

// Initialize ADC1 for oscillator
void ADCinit(void) {

    // GPIO PE0 = Analog in 13
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); // GPIO setup pin E0

    // Enable ADC1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Setup the ADC clock
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; //round up
    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);

    // Step configuration for ADC1
    ADCSequenceDisable(ADC1_BASE, 0); // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0); // specify the "Always" trigger
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);// in the 0th step, sample channel 3 (AIN3)
     // enable interrupt, and make it the end of sequence
    
    // Fire the interrupt for DMA and sequence now
    ADCSequenceEnable(ADC1_BASE, 0); // enable the sequence. it is now sampling
//    ADCIntEnable(ADC1_BASE, 0); // enable sequence 0 interrupt in the ADC1 peripheral
    ADCSequenceDMAEnable(ADC1_BASE, 0); // enable DMA for ADC1 sequence 0
    ADCIntEnableEx(ADC1_BASE, ADC_INT_DMA_SS0); // enable ADC1 sequence 0 DMA interrupt
//    IntPrioritySet(INT_ADC1SS0, ADC_INT_PRIORITY); // set ADC1 sequence 0 interrupt priority
//    IntEnable(INT_ADC1SS0); // enable ADC1 sequence 0 interrupt in int. controller
}

// Initialize DMA mode for ADC1
void ADCinit_DMA(void) {

    // Enable DMA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    uDMAEnable();
    uDMAControlBaseSet(gDMAControlTable);
    uDMAChannelAssign(UDMA_CH24_ADC1_0); // assign DMA channel 24 to ADC1 sequence 0
    uDMAChannelAttributeDisable(UDMA_SEC_CHANNEL_ADC10, UDMA_ATTR_ALL);

    // primary DMA channel = first half of the ADC buffer
    uDMAChannelControlSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT,
                         UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_4);
    uDMAChannelTransferSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT,
                         UDMA_MODE_PINGPONG, (void*)&ADC1_SSFIFO0_R,
                         (void*)&_adc.gADCBuffer[0], ADC_BUFFER_SIZE>>1);

    // alternate DMA channel = second half of the ADC buffer
    uDMAChannelControlSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_ALT_SELECT,
                         UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_4);
    uDMAChannelTransferSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_ALT_SELECT,
                         UDMA_MODE_PINGPONG, (void*)&ADC1_SSFIFO0_R,
                         (void*)&_adc.gADCBuffer[ADC_BUFFER_SIZE>>1], ADC_BUFFER_SIZE>>1);

    // Done initialize, enable now
    uDMAChannelEnable(UDMA_SEC_CHANNEL_ADC10);
}


// ISR for DMA
void ADC_ISR(void) {
    ADCIntClearEx(ADC1_BASE, ADC_INT_DMA_SS0); // clear the ADC1 sequence 0 DMA interrupt flag

    // Check the primary DMA channel for end of transfer, and restart if needed.
    if (uDMAChannelModeGet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT) == UDMA_MODE_STOP) {
        
        // restart the primary channel (same as setup)
        uDMAChannelTransferSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT,
                         UDMA_MODE_PINGPONG, (void*)&ADC1_SSFIFO0_R,
                         (void*)&_adc.gADCBuffer[0], ADC_BUFFER_SIZE>>1);
        gDMAPrimary = false; // DMA is currently occurring in the alternate buffer
    }

   // Check the alternate DMA channel for end of transfer, and restart if needed.
    if (uDMAChannelModeGet(UDMA_SEC_CHANNEL_ADC10 | UDMA_ALT_SELECT) == UDMA_MODE_STOP) {

        // restart the alternate channel (same as setup)
        uDMAChannelTransferSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_ALT_SELECT,
                         UDMA_MODE_PINGPONG, (void*)&ADC1_SSFIFO0_R,
                         (void*)&_adc.gADCBuffer[ADC_BUFFER_SIZE>>1], ADC_BUFFER_SIZE>>1);
        gDMAPrimary = true; // DMA is currently occurring in the primary buffer
    }
    
    // The DMA channel may be disabled if the CPU is paused by the debugger.
    if (!uDMAChannelIsEnabled(UDMA_SEC_CHANNEL_ADC10)) {
        uDMAChannelEnable(UDMA_SEC_CHANNEL_ADC10); // re-enable the DMA channel
    }
}

// Helper function to get the current ADC buffer index with DMA
uint32_t adc_get_buffer_index(void) {

    // Setup some variables for return and entering gate
    int32_t index;
    IArg key;

    // Entering critical section here, let's enter the gate and retrieve the key to leave
    key = GateTask_enter(gateTask0);

    // Getting the index for DMA is currently in the primary channel case
    if (gDMAPrimary) { 
        index = ADC_BUFFER_SIZE/2 - 1 -
                uDMAChannelSizeGet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT);
    }

    // Getting the index for DMA is currently in the alternate channel case
    else {
        index = ADC_BUFFER_SIZE - 1 -
                uDMAChannelSizeGet(UDMA_SEC_CHANNEL_ADC10 | UDMA_ALT_SELECT);
    }

    // Done with the critical section, leave the gate with the key then return the index
    GateTask_leave(gateTask0, key);
    return index;
}

// Searching for the trigger, rising = 1 for rising edge, rising = 0 for falling edge
uint32_t adc_trigger_search(uint16_t pTrigger, uint8_t rising) {

    // Choose a starting point for our buffer search
    uint32_t ADCBufferIndex = adc_get_buffer_index();
    int32_t start_index = ADC_BUFFER_WRAP(ADCBufferIndex - HALF_SCREEN_SIZE);
    int32_t search_index = start_index;

    // Start looking for the trigger value
    uint16_t drop_condition = ADC_BUFFER_SIZE >> 1;
    uint16_t current_value = _adc.gADCBuffer[search_index];
    uint16_t last_value = _adc.gADCBuffer[ADC_BUFFER_WRAP(search_index + 1)];
    uint16_t i = 0;

    // Looking for trigger position
    for (i = 0; i < drop_condition; i++) {
        search_index = ADC_BUFFER_WRAP(search_index - 1);
        last_value = current_value;
        current_value = _adc.gADCBuffer[search_index];
        if ((rising == 1) && (current_value >= pTrigger) && (last_value < pTrigger))
            return search_index;
        else if  ((rising == 0) && (current_value <= pTrigger) && (last_value > pTrigger))
            return search_index;
    }

    // Well we failed, return the start index
    return start_index;
}

// Copy samples half a screen behind and half a screen ahead of the trigger point
void adc_copy_buffer_samples(void) {

    // Get the current buffer index
    uint32_t ADCBufferIndex = adc_get_buffer_index();

    // Accessing the screen global variables from display, wrapping around semaphore pend and post
    Semaphore_pend(sem_accessDisplay, BIOS_WAIT_FOREVER);
    uint16_t pTrigger = _disp.pTrigger;
    uint8_t rising = _disp.rising;
    
    // Preparing the variable for copy
    uint8_t dispMode = _disp.dispMode;
    uint16_t iteration = 0;
    uint16_t start_pos = 0;

    // Choosing how much to copy based on mode of operation
    switch(dispMode) {
        case 0:
            iteration = SPECTRUM_SCREEN_SIZE; // 1024 samples
            start_pos = ADC_BUFFER_WRAP(ADCBufferIndex - SPECTRUM_SCREEN_SIZE); // 1024 sapmles behind the latest sample
            break;
        case 1:
            iteration = FULL_SCREEN_SIZE; // full screen 128 samples
            start_pos = ADC_BUFFER_WRAP(adc_trigger_search(pTrigger, rising) - HALF_SCREEN_SIZE); // half screen behind the trigger point
            break;
    }

    // Preparing variable for loop iteration
    uint16_t current_index = 0;
    int i;

    // Start copying over to our local buffer, wrap around semaphore again because why not
    for (i = 0; i < iteration; i++) {
        current_index = ADC_BUFFER_WRAP(start_pos + i);
        _disp.rawScreenBuffer[i] = _adc.gADCBuffer[current_index];
    }

    // Finish accessing the screen global variables, releasing now
    Semaphore_post(sem_accessDisplay);
}

// Scaling the ADC sample in the vertical direction
uint16_t adc_y_scaling(float fVoltsPerDiv, uint16_t sample) {
    float fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);
    return LCD_VERTICAL_MAX/2 - (int)roundf(fScale * ((int)sample - ADC_OFFSET));
}

// Waveform task to search for the trigger position
void adc_waveform_task(void) {
    while(1) {
        // Pending to get unblock
        Semaphore_pend(sem_waveformSignal ,BIOS_WAIT_FOREVER);

        // Start copying buffer sample over
        adc_copy_buffer_samples();

        // Done copying, post to the processing task
        Semaphore_post(sem_processingSignal);

    }
}

// Processing task for normal mode and spectrum mode
void adc_process_task(void) {

    // Allocating variables for FFT calculation
    static char kiss_fft_cfg_buffer[KISS_FFT_CFG_SIZE]; // Kiss FFT config memory
    size_t buffer_size = KISS_FFT_CFG_SIZE;
    kiss_fft_cfg cfg; // Kiss FFT config
    static kiss_fft_cpx in[NFFT], out[NFFT]; // complex waveform and spectrum buffers
    cfg = kiss_fft_alloc(NFFT, 0, kiss_fft_cfg_buffer, &buffer_size); // init Kiss FFT

    // Start the process task infinite loop here
    while(1) {
        // Pend on a semaphore until unblocked
        Semaphore_pend(sem_processingSignal, BIOS_WAIT_FOREVER);

        // Pend on semaphore before accessing the global variable
        Semaphore_pend(sem_accessDisplay, BIOS_WAIT_FOREVER);

        // Clear now, start the computational process
        uint8_t dispMode = _disp.dispMode;
        uint16_t i = 0;
        float fVoltageScale = gVoltageScale[_disp.voltsPerDivPointer];
        switch(dispMode) { // calculations based on mode of operation
            case 0: // spectrum mode
            
                // Computing FFT and output to out array    
                for (i = 0; i < NFFT; i++) { // generate an input waveform
                     in[i].r = _disp.rawScreenBuffer[i] - ADC_OFFSET; // real part of waveform
                     in[i].i = 0; // imaginary part of waveform
                }
                kiss_fft(cfg, in, out); // compute FFT

                // Copying to screen buffer
                for (i = 0; i < FULL_SCREEN_SIZE; i++) {
                    _disp.scaledScreenBuffer[i] = 128 - roundf(10*log10f((out[i].i*out[i].i) + (out[i].r*out[i].r)));
                }
                break;

            case 1: // normal mode
                for (i = 0; i < FULL_SCREEN_SIZE; i++) {
                    _disp.scaledScreenBuffer[i] = adc_y_scaling(fVoltageScale, _disp.rawScreenBuffer[i]);
                }
                break;
        }

        // Done with the calculations, post to semaphore again
        Semaphore_post(sem_accessDisplay);

        // Done computation, signal the display task to display
        Semaphore_post(sem_dispUpdate);

        // Then signal the ADC to capture some more buffer
        Semaphore_post(sem_waveformSignal);
    }
}
