/*
 * lcd_display.c
 *
 *  Created on: Mar 22, 2018
 *      Author: hpnguyen
 *
 *  ECE3849 Lab 1 drawing to LCD for TM4C1294
 */

// Standard C libraries
#include <stdint.h>
#include <stdio.h>

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

    // Setting graphic context and font
    tContext sContext;
    GrContextInit(&sContext, &g_sCrystalfontz128x128);
    GrContextFontSet(&sContext, &g_sFontFixed6x8); 

    // Full-screen rectangle
//    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};
}

// Plot the function based on the local buffer
void lcd_plot_func(float fVoltsPerDiv, float fTimePerDiv) {

    // String buffer to draw out to screen and index int
    char str[128];
    int i = 0;

    // Initialize the screen first
    Crystalfontz128x128_Init(); // Initialize the LCD display driver
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP); // set screen orientation
    tContext sContext;
    GrContextInit(&sContext, &g_sCrystalfontz128x128); // Initialize the grlib graphics context
    GrContextFontSet(&sContext, &g_sFontFixed6x8); // select font
    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1}; // full-screen rectangle

    // Preparing the screen background and text format
    GrContextForegroundSet(&sContext, ClrBlack);
    GrRectFill(&sContext, &rectFullScreen); // fill screen with black
    GrContextForegroundSet(&sContext, ClrYellow); // yellow text

    // Put in the grid and display 
//    lcd_draw_grid();
//    lcd_display_choices(char);

    // Iterate through the buffer and draw out points to screen
    for (i = 0; i < FULL_SCREEN_SIZE; i++) {
        uint16_t x = i;
        uint16_t y = adc_y_scaling(fVoltsPerDiv, gScreenBuffer[i]);
        snprintf(str, sizeof(str), ".");
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ x, /*y*/ y, /*opaque*/ false);
    }

    // Flush out to screen
    GrFlush(&sContext); // flush the frame buffer to the LCD
}

// Drawing grid onto the LCD screen
void lcd_draw_grid() {

}
