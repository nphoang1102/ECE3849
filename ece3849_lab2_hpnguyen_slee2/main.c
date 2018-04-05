/*
 * ECE 3849 Lab2 starter project
 *
 * Gene Bogdanov    9/13/2017
 */

/* Importing header files */

// Import local header file
#include "RTOS_helper.h"


uint32_t gSystemClock = 120000000; // [Hz] system clock frequency

void task0_func(UArg arg1, UArg arg2);
void ButtonClockSignal(void);
void ButtonTask(UArg arg0, UArg arg1);

/*
 *  ======== main ========
 */
int main(void)
{
    IntMasterDisable();

    // hardware initialization goes here

    /* Start BIOS */
    BIOS_start();

    return (0);
}

// ======== main.c from lab1 starts here ========
/*
 *
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
#include "timer.h"

// Declaring global variables
uint32_t gSystemClock; // [Hz] system clock frequency
volatile uint32_t gTime = 0; // time in hundredths of a second

// Importing global variables
extern volatile uint32_t gButtons;

int main(void)
{
    // Disable global interrupt before we do any initialize
    IntMasterDisable();

    // Local variables
    uint8_t rising = 1; // default rising edge trigger
    uint16_t pTrigger = 2048; // default trigger point set to 0V
    uint8_t voltsPerDivPointer = 3; // default volts per grid is 1V
    uint16_t time_scale = 20; // default time scale per grid is 20us
    float cpu_load = 0.0; // CPU load average over 10ms time interval

    // Enable the Floating Point Unit, and permit ISRs to use it
    FPUEnable();
    FPULazyStackingEnable();

    // Initialize the system clock to 120 MHz
    gSystemClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);

    // Initialization here
    timer_oneshot_init();
    lcd_init();
    ButtonInit(); // setup the ISR for our counter, will not need under RTOS anymore

    // Get unloaded tick count over 10ms and allocate memory for loaded tick count
    uint32_t count_unloaded = timer_load_count();
    uint32_t count_loaded = 0;

    // Init the ADC later or it will miss the first deadline...
    ADCinit();

    // Enable global interrupt
    IntMasterEnable(); // now that we finished setting things up, re-enable interrupts

    while (true) {

        // Handling button input from user
        ButtonHandling(&rising, &voltsPerDivPointer, &time_scale);

        // Copy the ADC buffer value into the screen
        adc_copy_buffer_samples(pTrigger, rising);

        // Display everything onto the screen
        lcd_show_screen(voltsPerDivPointer, time_scale, cpu_load, rising);

        // Now that we are done with the iteration, start measuring load
        count_loaded = timer_load_count();
        cpu_load = (1.0f - (float)count_loaded/count_unloaded) * 100.0f;
    }
}
 *
 */

