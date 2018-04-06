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
    0.0 // CPU starts at no load
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
    lcd_plot_func(gVoltageScale[_disp.voltsPerDivPointer], &sContext);
    lcd_draw_text(&sContext, _disp.time_scale, _disp.voltsPerDivPointer, _disp.cpu_load, _disp.pTrigger);

    // Flush out to screen
    GrFlush(&sContext);
}

// Plot the function based on the local buffer
void lcd_plot_func(float fVoltsPerDiv, tContext * sContext) {

    // Iteration index
    int i = 0;

    // Yellow function
    GrContextForegroundSet(sContext, ClrYellow);

    // Starting point
    uint16_t x = 0;
    uint16_t y = adc_y_scaling(fVoltsPerDiv, _adc.gScreenBuffer[0]);
    GrPixelDraw(sContext, /*x*/ x, /*y*/ y);

    // Iterate through the rest of the buffer and draw out lines to screen
    for (i = 1; i < FULL_SCREEN_SIZE; i++) {

        // Copy over the last x,y values so we can draw a line
        uint16_t last_x = x;
        uint16_t last_y = y;

        // Get the new x and y
        x = i;
        y = adc_y_scaling(fVoltsPerDiv, _adc.gScreenBuffer[i]);

        // Now draw the line from 2 points
        GrLineDraw(sContext, last_x, last_y, x, y );
    }
}

// Drawing grid onto the LCD screen
void lcd_plot_grid(tContext * sContext) {

    // Iteration index
    int i = 0;

    // Drawing the grids in dark blue
    GrContextForegroundSet(sContext, ClrMidnightBlue);
    for (i = 0; i < 7; i++) {
        GrLineDrawH(sContext, 0, 128, PIXELS_PER_DIV * i + 4);
        GrLineDrawV(sContext, PIXELS_PER_DIV * i + 4, 0, 128);
    }

    // Highlight the center axi
    GrContextForegroundSet(sContext, ClrBlue);
    GrLineDrawH(sContext, 0, 128, 64);
    GrLineDrawV(sContext, 64, 0, 128);
}

// Drawing text out on the LCD screen
void lcd_draw_text(tContext * sContext, uint16_t time_scale, uint8_t voltsPerDivPointer, float cpu_load, uint8_t trigger) {

    // String buffer to write text to
    char str[32];

    // White text
    GrContextForegroundSet(sContext, ClrWhite);

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
    snprintf(str, sizeof(str), "CPU load: %.1f%%", cpu_load);
    GrStringDraw(sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 120, /*opaque*/ false);
}
