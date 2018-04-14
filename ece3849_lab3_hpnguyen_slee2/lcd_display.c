/*
 * lcd_display.c
 *
 *  Created on: Mar 22, 2018
 *      Author: hpnguyen
 *
 *  ECE3849 Lab 1 drawing to LCD for TM4C1294
 */

// Standard C libraries
#include <stdio.h>
#include <string.h>

// Driver libraries from TivaWare
#include "Crystalfontz128x128_ST7735.h"

// Library from project
#include "lcd_display.h"
#include "adc.h"
#include "RTOS_helper.h"

// Declaring global variables
const float gVoltageScale[] = {0.1, 0.2, 0.5, 1.0}; // float value of the voltage scale
const char * const gVoltageScaleStr[] = {
    "100 mV", "200 mV", "500 mV", "1.00 V" //text value of the voltage scale
};

// Initializing global space for variable storage
struct Display _disp = {
    1, // default rising edge trigger
    2048, // default trigger point set to 0V
    3, // default volts per grid is 1V
    20, // default time scale per grid is 20us
    0.0, // CPU starts at no load
    1, // oscilloscope mode on startup
    {0}, // initialize the screen buffer before copying stuffs in here
    {0}
};

// Screen initialization
void lcd_init() {

    // Initialize the LCD display driver and set screen orientation
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
}

// Plot the whole oscillator screen
void lcd_show_screen(void) {

    // Create some pointers to fill in
    tContext sContext;
    GrContextInit(&sContext, &g_sCrystalfontz128x128); // Initialize the grlib graphics context
    GrContextFontSet(&sContext, &g_sFontFixed6x8); // select font
    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1}; // full-screen rectangle

    // Preparing the screen background and text format
    GrContextForegroundSet(&sContext, ClrBlack);
    GrRectFill(&sContext, &rectFullScreen); // fill screen with black

    // Plotting everything onto the screen and flush once
    lcd_plot_grid(&sContext);
    lcd_plot_func(&sContext);
    lcd_draw_text(&sContext);

    // Flush out to screen
    GrFlush(&sContext);
}

// Plot the function based on the local buffer
void lcd_plot_func(tContext * sContext) {

    // Iteration index
    int i = 0;

    // Access the current mode of operation to plot accordingly
    Semaphore_pend(sem_accessDisplay, BIOS_WAIT_FOREVER);
    uint8_t dispMode = _disp.dispMode;
    Semaphore_post(sem_accessDisplay);

    // Function color depending on current mode of operation
    switch (dispMode) {
        case 0: // Spectrum mode has orange function
            GrContextForegroundSet(sContext, ClrOrange);
            break;

        case 1: // Normal mode has yellow function
            GrContextForegroundSet(sContext, ClrYellow);
            break;
    }

    // Pend on the semaphore here before accessing the ADC shared data
    Semaphore_pend(sem_accessDisplay, BIOS_WAIT_FOREVER);

    // Starting point
    uint16_t x = 0;
    uint16_t y = _disp.scaledScreenBuffer[0];
    GrPixelDraw(sContext, /*x*/ x, /*y*/ y);

    // Iterate through the rest of the buffer and draw out lines to screen
    for (i = 1; i < FULL_SCREEN_SIZE; i++) {

        // Copy over the last x,y values so we can draw a line
        uint16_t last_x = x;
        uint16_t last_y = y;

        // Get the new x and y
        x = i;
        y = _disp.scaledScreenBuffer[i];

        // Now draw the line from 2 points
        GrLineDraw(sContext, last_x, last_y, x, y );
    }

    // Done with the variable accessing, post to semaphore now
    Semaphore_post(sem_accessDisplay);
}

// Drawing grid onto the LCD screen
void lcd_plot_grid(tContext * sContext) {

    // Iteration index
    int i = 0;

    // Access the current display mode to plot accordingly
    Semaphore_pend(sem_accessDisplay, BIOS_WAIT_FOREVER);
    uint8_t dispMode = _disp.dispMode;
    Semaphore_post(sem_accessDisplay);

    // Plot the grids depending on what mode of operation we're at
    switch(dispMode) {
        case 0: // spectrum mode
        
            // Drawing the grids in dark blue
            GrContextForegroundSet(sContext, ClrMidnightBlue);
            for (i = 0; i < 7; i++) {
                GrLineDrawH(sContext, 0, 128, PIXELS_PER_DIV * i);
                GrLineDrawV(sContext, PIXELS_PER_DIV * i, 0, 128);
            }

            // Highlight the center axi in light blue
            GrContextForegroundSet(sContext, ClrBlue);
            GrLineDrawH(sContext, 0, 128, 20);
            GrLineDrawV(sContext, 0, 0, 128);
            break;

        case 1: // normal oscilloscope mode
        
            // Drawing the grids in dark blue
            GrContextForegroundSet(sContext, ClrMidnightBlue);
            for (i = 0; i < 7; i++) {
                GrLineDrawH(sContext, 0, 128, PIXELS_PER_DIV * i + 4);
                GrLineDrawV(sContext, PIXELS_PER_DIV * i + 4, 0, 128);
            }
            // Highlight the center axi in light blue
            GrContextForegroundSet(sContext, ClrBlue);
            GrLineDrawH(sContext, 0, 128, 64);
            GrLineDrawV(sContext, 64, 0, 128);
            break;

    }

}

// Drawing text out on the LCD screen
void lcd_draw_text(tContext * sContext) {

    // String buffer to write text to
    char str[32];

    // White text
    GrContextForegroundSet(sContext, ClrWhite);

    // Acess some display params
    Semaphore_pend(sem_accessDisplay, BIOS_WAIT_FOREVER);
    uint16_t time_scale = _disp.time_scale;
    uint8_t voltsPerDivPointer = _disp.voltsPerDivPointer;
//    float cpu_load = _disp.cpu_load;
    uint8_t trigger = _disp.rising;
    uint8_t dispMode = _disp.dispMode;
    Semaphore_post(sem_accessDisplay);

    // Print out text to the screen depending on what mode of operation we're doing
    switch(dispMode) {
        case 0: // spectrum mode

            // Print out the frequency scale
            snprintf(str, sizeof(str), "20 kHz");
            GrStringDraw(sContext, str, /*length*/ -1, /*x*/ 5, /*y*/ 0, /*opaque*/ false);

            // Print out the dB scale
            snprintf(str, sizeof(str), "20 dB");
            GrStringDraw(sContext, str, /*length*/ -1, /*x*/ 50, /*y*/ 0, /*opaque*/ false);

            break;

        case 1: // default oscilloscope mode of operation

            // Print out the time scale
            snprintf(str, sizeof(str), "%u us", time_scale);
            GrStringDraw(sContext, str, /*length*/ -1, /*x*/ 5, /*y*/ 0, /*opaque*/ false);

            // Print out the voltage scale
            snprintf(str, sizeof(str), "%s", gVoltageScaleStr[voltsPerDivPointer]);
            GrStringDraw(sContext, str, /*length*/ -1, /*x*/ 50, /*y*/ 0, /*opaque*/ false);

            // Print out the trigger slope
            switch(trigger) {
            case 1: // rising edge trigger
                GrLineDrawH(sContext, 114, 121, 0);
                GrLineDrawV(sContext, 114, 0, 7);
                GrLineDrawH(sContext, 107, 114, 7);
                GrLineDraw(sContext, 114, 2, 111, 5);
                GrLineDraw(sContext, 114, 2, 117, 5);
                break;
            case 0: // falling edge trigger
                GrLineDrawH(sContext, 107, 114, 0);
                GrLineDrawV(sContext, 114, 0, 7);
                GrLineDrawH(sContext, 114, 121, 7);
                GrLineDraw(sContext, 114, 5, 111, 2);
                GrLineDraw(sContext, 114, 5, 117, 2);
                break;
            }

            // Print out the CPU load
            // snprintf(str, sizeof(str), "CPU load: %.1f%%", cpu_load);
            // GrStringDraw(sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 120, /*opaque*/ false);
            
            break;
    }
}

// Display task
void lcd_display_task(void) {
    while(1) {
        // Pend on a semaphore until unblocked
        Semaphore_pend(sem_dispUpdate, BIOS_WAIT_FOREVER);

        // We're clear, proceed to display
        lcd_show_screen();
    }
}
