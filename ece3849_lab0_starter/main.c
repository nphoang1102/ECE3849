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

uint32_t gSystemClock; // [Hz] system clock frequency
volatile uint32_t gTime = 8345; // time in hundredths of a second

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

    uint32_t time;  // local copy of gTime
    char str[128];   // string buffer
    // full-screen rectangle
    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};

    // LCD init done, move on to ISR
    ButtonInit(); // setup the ISR for our counter
    IntMasterEnable(); // now that we finished setting things up, re-enable interrupts

    while (true) {
        /* Converting the integer to binary values for the button states */
        uint8_t bit0 = (gButtons & (1 << 0)) ? 1 : 0;
        uint8_t bit1 = (gButtons & (1 << 1)) ? 1 : 0;
        uint8_t bit2 = (gButtons & (1 << 2)) ? 1 : 0;
        uint8_t bit3 = (gButtons & (1 << 3)) ? 1 : 0;
        uint8_t bit4 = (gButtons & (1 << 4)) ? 1 : 0;
        uint8_t bit5 = (gButtons & (1 << 5)) ? 1 : 0;
        uint8_t bit6 = (gButtons & (1 << 6)) ? 1 : 0;
        uint8_t bit7 = (gButtons & (1 << 7)) ? 1 : 0;
        uint8_t bit8 = (gButtons & (1 << 8)) ? 1 : 0;



        GrContextForegroundSet(&sContext, ClrBlack);
        GrRectFill(&sContext, &rectFullScreen); // fill screen with black
        time = gTime; // read shared global only once
        snprintf(str, sizeof(str), "Time = %02u:%02u:%02u", ((time/6000)%60), ((time/100)%60), (time%100)); // convert time to string
        GrContextForegroundSet(&sContext, ClrYellow); // yellow text
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false);

        snprintf(str, sizeof(str), "%u%u%u%u%u%u%u%u%u", bit8, bit7, bit6, bit5, bit4, bit3, bit2, bit1, bit0); // convert time to string
        GrContextForegroundSet(&sContext, ClrYellow); // yellow text
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 16, /*opaque*/ false);

        GrFlush(&sContext); // flush the frame buffer to the LCD
    }
}
