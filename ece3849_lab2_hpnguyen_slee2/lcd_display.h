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

// Plotting constant
#define PIXELS_PER_DIV 20

// Struct definition for global variable storage
struct Display {
    uint8_t rising; // Trigger edge, 1 for rising, 0 for falling
    uint16_t pTrigger; // Trigger point value in ADC bit space
    uint8_t voltsPerDivPointer; // Pointer to the volts per division array
    uint16_t time_scale; // Time scale per grid
    float cpu_load; // CPU load average over 10ms time interval
};
extern struct Display _disp;

/* Function prototypes */
void lcd_init(); // initialize the LCD screen
void lcd_show_screen(void); // show the complete oscillator screen
void lcd_plot_func(float fVoltsPerDiv, tContext * sContext); // plotting the function to LCD screen
void lcd_plot_grid(tContext * sContext); // plot the grid
void lcd_draw_text(tContext * sContext, uint16_t time_scale, uint8_t voltsPerDivPointer, float cpu_load, uint8_t trigger); // draw the text

#endif /* LCD_DISPLAY_H_ */
