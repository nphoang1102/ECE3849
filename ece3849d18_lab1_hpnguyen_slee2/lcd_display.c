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

// Importing global variables from other modules
extern uint16_t gScreenBuffer[FULL_SCREEN_SIZE];

// Screen initialization
void lcd_init() {

    // Initialize the LCD display driver and set screen orientation
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
}

// Plot the whole oscillator screen
void lcd_show_screen(float fVoltsPerDiv, uint16_t time_scale, uint16_t voltage_scale, float cpu_load, uint8_t trigger) {

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
    lcd_plot_func(fVoltsPerDiv, &sContext);
    lcd_draw_text(&sContext, time_scale, voltage_scale, cpu_load, trigger);

    // Flush out to screen
    GrFlush(&sContext);
}

// Plot the function based on the local buffer
void lcd_plot_func(float fVoltsPerDiv, tContext * sContext) {

    // Iteration index
    int i = 0;

    // Yellow function
    GrContextForegroundSet(sContext, ClrYellow);

    // Iterate through the buffer and draw out points to screen
    for (i = 0; i < FULL_SCREEN_SIZE; i++) {
        uint16_t x = i;
        uint16_t y = adc_y_scaling(fVoltsPerDiv, gScreenBuffer[i]);
        GrPixelDraw(sContext, /*x*/ x, /*y*/ y);
    }
}

// Drawing grid onto the LCD screen
void lcd_plot_grid(tContext * sContext) {

    // Iteration index
    int i = 0;
//
//    // Drawing the grids in dark blue
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
void lcd_draw_text(tContext * sContext, uint16_t time_scale, uint16_t voltage_scale, float cpu_load, uint8_t trigger) {

    // String buffer to write text to
    char str[32];

    // White text
    GrContextForegroundSet(sContext, ClrWhite);

    // Print out the time scale
    snprintf(str, sizeof(str), "%u us", time_scale);
    GrStringDraw(sContext, str, /*length*/ -1, /*x*/ 5, /*y*/ 0, /*opaque*/ false);

    // Print out the voltage scale
    snprintf(str, sizeof(str), "%u mV", voltage_scale);
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
