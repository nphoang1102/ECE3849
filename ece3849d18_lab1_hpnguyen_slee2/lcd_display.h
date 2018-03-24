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

/* Macro definition */

// Screen constants
#define FULL_SCREEN_SIZE 128 // self explanatory
#define HALF_SCREEN_SIZE 64 // half of 128 pixels

// Plotting constant
#define PIXELS_PER_DIV 20

/* Function prototypes */
void lcd_init(); // initialize the LCD screen
void lcd_show_screen(float fVoltsPerDiv, uint16_t time_scale, uint16_t voltage_scale, float cpu_load); // show the complete oscillator screen
void lcd_plot_func(float fVoltsPerDiv, tContext * sContext); // plotting the function to LCD screen
void lcd_plot_grid(tContext * sContext); // plot the grid
void lcd_draw_text(tContext * sContext, uint16_t time_scale, uint16_t voltage_scale, float cpu_load); // draw the text
uint8_t *lcd_read_image(char *fname); // funtion to draw the rising edge trigger symbol

#endif /* LCD_DISPLAY_H_ */
