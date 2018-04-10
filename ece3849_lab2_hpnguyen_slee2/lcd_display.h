/*
 * lcd_display.h
 *
 *  Created on: Mar 22, 2018
 *      Author: hpnguyen
 *
 *  Header for lcd_display.c for drawing helper functions
 */

#ifndef LCD_DISPLAY_H_
#define LCD_DISPLAY_H_

#include <stdint.h>
#include "Crystalfontz128x128_ST7735.h"

/* Macro definition */

// Screen constants
#define FULL_SCREEN_SIZE 128 // self explanatory
#define HALF_SCREEN_SIZE 64 // half of 128 pixels
#define SPECTRUM_SCREEN_SIZE 1024 // number of readings required for spectrum mode

// Plotting constant
#define PIXELS_PER_DIV 20

// Struct definition for global variable storage
struct Display {
    uint8_t rising; // Trigger edge, 1 for rising, 0 for falling
    uint16_t pTrigger; // Trigger point value in ADC bit space
    uint8_t voltsPerDivPointer; // Pointer to the volts per division array
    uint16_t time_scale; // Time scale per grid
    float cpu_load; // CPU load average over 10ms time interval
    uint8_t dispMode; // Display mode, 1 for normal oscilloscope, 0 for spectrum mode
    uint16_t rawScreenBuffer[SPECTRUM_SCREEN_SIZE]; // raw copy of the screen buffer
    uint16_t scaledScreenBuffer[FULL_SCREEN_SIZE]; // output of the screen buffer after processed

};
extern struct Display _disp;

/* Function prototypes */
void lcd_init(); // initialize the LCD screen
void lcd_show_screen(void); // show the complete oscillator screen
void lcd_plot_func(tContext * sContext); // plotting the function to LCD screen
void lcd_plot_grid(tContext * sContext); // plot the grid
void lcd_draw_text(tContext * sContext); // draw the text

#endif /* LCD_DISPLAY_H_ */
