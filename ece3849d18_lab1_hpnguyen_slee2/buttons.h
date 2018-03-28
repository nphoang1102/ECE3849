/*
 * buttons.h
 *
 *  Created on: Aug 12, 2012, modified 9/8/2017
 *      Author: Gene Bogdanov
 *
 *  Button debouncer, calibrated for 200 samples/sec.
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

/* Macro definition */

// Button debounce macro
#define BUTTON_COUNT 5				// number of buttons excluding joystick directions
#define BUTTON_AND_JOYSTICK_COUNT 9 // number of buttons including joystick directions
#define BUTTON_SAMPLES_PRESSED 2	// number of samples before a button is considered pressed
#define BUTTON_SAMPLES_RELEASED 5	// number of samples before a button is considered released

// Autorepeat condition
#define BUTTON_AUTOREPEAT_INITIAL 100   // how many samples must read pressed before autorepeat starts
#define BUTTON_AUTOREPEAT_NEXT 10       // how many samples must read pressed before the next repetition

// Joystick ADC read value
#define JOYSTICK_UPPER_PRESS_THRESHOLD 3595     // above this ADC value, button is pressed
#define JOYSTICK_UPPER_RELEASE_THRESHOLD 3095   // below this ADC value, button is released
#define JOYSTICK_LOWER_PRESS_THRESHOLD 500      // below this ADC value, button is pressed
#define JOYSTICK_LOWER_RELEASE_THRESHOLD 1000   // above this ADC value, button is released

// Timer interrupt macro
#define BUTTON_SCAN_RATE 200    // [Hz] button scanning interrupt rate
#define BUTTON_INT_PRIORITY 32  // button interrupt priority (higher number is lower priority)

// counter value indicating button pressed state
#define BUTTON_PRESSED_STATE (BUTTON_SAMPLES_RELEASED*BUTTON_SAMPLES_PRESSED)
#define BUTTON_STATE_INCREMENT (BUTTON_PRESSED_STATE/BUTTON_SAMPLES_PRESSED)
#define BUTTON_STATE_DECREMENT (BUTTON_PRESSED_STATE/BUTTON_SAMPLES_RELEASED)

// Button FIFO queue helpers
#define BUTTON_QUEUE_LENGTH 16
#define BUTTON_BUFFER_WRAP(i) ((i) & (BUTTON_QUEUE_LENGTH - 1))

// Global variable
extern volatile uint32_t gButtons;	// debounced button state, one per bit in the lowest bits
extern uint32_t gJoystick[2];       // joystick coordinates
extern uint32_t gADCSamplingRate;   // [Hz] actual ADC sampling rate

/* Function prototypes */
void ButtonInit(void); // initialize all button and joystick handling hardware
void ButtonDebounce(uint32_t buttons); // update the debounced button state in the global variable gButtons the input argument is a bitmap of raw button state from the hardware
void ButtonReadJoystick(void); // sample joystick and convert to button presses
uint32_t ButtonAutoRepeat(void); // autorepeat button presses if a button is held long enough
int ButtonPutQ(uint32_t button_bitmap); // putting button read into FIFO queue
int ButtonGetQ(uint32_t *button_state); // getting button read out of the FIFO queue
int ButtonHandling(uint8_t *rising, uint8_t *voltsPerDivPointer, uint16_t *time_scale); // button handling from user input

#endif /* BUTTONS_H_ */
