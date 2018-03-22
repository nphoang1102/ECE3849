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
#include "buttons.h"
#include "adc.h"

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

    Crystalfontz128x128_Init(); // Initialize the LCD display driver
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP); // set screen orientation

    tContext sContext;
    GrContextInit(&sContext, &g_sCrystalfontz128x128); // Initialize the grlib graphics context
    GrContextFontSet(&sContext, &g_sFontFixed6x8); // select font

    // Local variable
    uint32_t time;  // local copy of gTime
    uint32_t butt;
    char str[128];   // string buffer
    
    // full-screen rectangle
    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};

    // LCD init done, move on to ISR
    ButtonInit(); // setup the ISR for our counter
//    ADCinit();
    IntMasterEnable(); // now that we finished setting things up, re-enable interrupts

    while (true) {
        // Read one and store necessary global volatile variables
        butt = gButtons;
        time = gTime;

        // Preparing the screen background and text format first
        GrContextForegroundSet(&sContext, ClrBlack);
        GrRectFill(&sContext, &rectFullScreen); // fill screen with black
        GrContextForegroundSet(&sContext, ClrYellow); // yellow text

        // Set what text to print out and its respective location
        snprintf(str, sizeof(str), "Time = %02u:%02u:%02u", ((time/6000)%60), ((time/100)%60), (time%100)); // convert time to string
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false);
        snprintf(str, sizeof(str), "Button states:");
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 16, /*opaque*/ false);
        snprintf(str, sizeof(str), "%u%u%u%u%u%u%u%u%u", (butt>>8)&1, (butt>>7)&1, (butt>>6)&1, (butt>>5)&1, (butt>>4)&1, (butt>>3)&1, (butt>>2)&1, (butt>>1)&1, (butt>>0)&1);
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 32, /*opaque*/ false);
        
        // Flush out to screen
        GrFlush(&sContext); // flush the frame buffer to the LCD
    }
}
