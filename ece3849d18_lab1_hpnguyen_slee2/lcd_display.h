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


/* Function prototypes */
void lcd_init(); // initialize the LCD screen
void lcd_plot_func(float fVoltsPerDiv, float fTimePerDiv); // plotting the function to LCD screen

#endif /* LCD_DISPLAY_H_ */
