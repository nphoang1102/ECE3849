/*
 * ECE 3849 Lab2 starter project
 *
 * Gene Bogdanov    9/13/2017
 */

/* Importing header files */

// Some existed library
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>

// Import local header file
#include "adc.h"
#include "buttons.h"
#include "lcd_display.h"
#include "RTOS_helper.h"
#include "timer.h"

// Global variable
uint32_t gSystemClock = 120000000; // [Hz] system clock frequency


/*
 *  ======== main ========
 */
int main(void)
{
    IntMasterDisable();

    /* Hardware initialization goes here */
    lcd_init();
    ButtonInit(); // setup the ISR for our counter, will not need under RTOS anymore
    ADCinit();

    /* Start BIOS */
    BIOS_start();

    /* DO NOT PUT ANY CODE BELOW THIS, AS THE BIOS_start() IS A FOREVER LOOP */

    return (0);
}
