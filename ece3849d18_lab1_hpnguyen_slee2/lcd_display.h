/*
 * lcd_display.h
 *
 *  Created on: Mar 22, 2018
 *      Author: hpnguyen
 *
 *  Header for lcd_display.c for drawing helper functions
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>


/* Function prototypes */
void lcd_init(); // initialize the LCD screen
void lcd_plot_func(float fVoltsPerDiv, float fTimePerDiv); // plotting the function to LCD screen

#endif /* ADC_H_ */