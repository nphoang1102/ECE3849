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

uint32_t gSystemClock; // [Hz] system clock frequency
volatile uint32_t gTime = 0; // time in hundredths of a second

extern volatile uint32_t gButtons;

int main(void)
{
    IntMasterDisable();

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
        adc_copy_buffer_samples(0, 1);
        lcd_show_screen(1.0f, 20, 200, 60.1f, 1);
    }
}
