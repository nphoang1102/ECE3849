/**
 * main.c
 *
 * ECE 3849 Lab 0 Starter Project
 * Gene Bogdanov    10/18/2017
 *
 * This version is using the new hardware for B2017: the EK-TM4C1294XL LaunchPad with BOOSTXL-EDUMKII BoosterPack.
 *
 */

// Some existed library
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>

// Button for handling debounce and ISR
#include "adc.h"
#include "buttons.h"
#include "lcd_display.h"

// Declaring global variables
uint32_t gSystemClock; // [Hz] system clock frequency
volatile uint32_t gTime = 0; // time in hundredths of a second

// Importing global variables
extern volatile uint32_t gButtons;
extern const float gVoltageScale[];

int main(void)
{
    // Disable global interrupt before we do any initialize
    IntMasterDisable();

    // Local variables
    uint8_t rising = 1; // default rising edge trigger
    uint16_t pTrigger = 2048; // default trigger point set to 0V
    uint8_t voltsPerDivPointer = 3; // default volts per grid is 1V
    uint16_t time_scale = 20; // default time scale per grid is 20us
    uint16_t voltage_scale = 200; // default voltage scale is 200mV
    float cpu_load = 60.1; // initialize to 60.1% because why not

    // Enable the Floating Point Unit, and permit ISRs to use it
    FPUEnable();
    FPULazyStackingEnable();

    // Initialize the system clock to 120 MHz
    gSystemClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);
    
    // Initialization here
    lcd_init();
    ButtonInit(); // setup the ISR for our counter
    ADCinit();

    // Enable global interrupt
    IntMasterEnable(); // now that we finished setting things up, re-enable interrupts

    while (true) {

        // Handling button input from user
        ButtonHandling(&rising, &voltsPerDivPointer, &time_scale);

        // Copy the ADC buffer value into the screen
        adc_copy_buffer_samples(pTrigger, rising);

        // Display everything onto the screen
        lcd_show_screen(gVoltageScale[voltsPerDivPointer], time_scale, voltage_scale, cpu_load, rising);
    }
}
