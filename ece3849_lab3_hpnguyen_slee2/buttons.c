/*
 * buttons.c
 *
 *  Created on: Aug 12, 2012, modified 9/8/2017
 *      Author: Gene Bogdanov
 *
 * ECE 3849 Lab button handling
 */

// Standard C libraries
#include <stdint.h>
#include <stdbool.h>

// TivaWare driver libraries
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "sysctl_pll.h"

// Libraries from project
#include "lcd_display.h"
#include "buttons.h"
#include "adc.h"
#include "RTOS_helper.h"

// Initialize global space for variable storage
struct Button _butt = {
    0, // debounced button state, one per bit in the lowest bits
       // button is pressed if its bit is 1, not pressed if 0
    {0}, // joystick coordinates
};

// imported globals
extern uint32_t gSystemClock;   // [Hz] system clock frequency

// initialize all button and joystick handling hardware
void ButtonInit(void) {
    // initialize a general purpose timer for periodic interrupts
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
//    TimerDisable(TIMER0_BASE, TIMER_BOTH);
//    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
//    TimerLoadSet(TIMER0_BASE, TIMER_A, (float)gSystemClock / BUTTON_SCAN_RATE - 0.5f);
//    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
//    TimerEnable(TIMER0_BASE, TIMER_BOTH);

    // initialize interrupt controller to respond to timer interrupts
//    IntPrioritySet(INT_TIMER0A, BUTTON_INT_PRIORITY);
//    IntEnable(INT_TIMER0A);

    // GPIO PJ0 and PJ1 = EK-TM4C1294XL buttons 1 and 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // GPIO PH1 = BoosterPack button S1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    GPIOPinTypeGPIOInput(GPIO_PORTH_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTH_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // GPIO PK6 = BoosterPack button S2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_6);
    GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // analog input AIN13, at GPIO PD2 = BoosterPack Joystick HOR(X)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_2);

    // analog input AIN17, at GPIO PK1 = BoosterPack Joystick VER(Y)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    GPIOPinTypeADC(GPIO_PORTK_BASE, GPIO_PIN_1);

    // GPIO PD4 = BoosterPack Joystick Select button
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // initialize ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; // round divisor up
    _butt.gADCSamplingRate = pll_frequency / (16 * pll_divisor); // actual sampling rate may differ from ADC_SAMPLING_RATE
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor); // only ADC0 has PLL clock divisor control

    // initialize ADC sampling sequence
    ADCSequenceDisable(ADC0_BASE, 0);
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH13);                             // Joystick HOR(X)
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH17 | ADC_CTL_IE | ADC_CTL_END);  // Joystick VER(Y)
    ADCSequenceEnable(ADC0_BASE, 0);
}

// update the debounced button state gButtons
void ButtonDebounce(uint32_t buttons)
{
	int32_t i, mask;
	static int32_t state[BUTTON_COUNT]; // button state: 0 = released
									    // BUTTON_PRESSED_STATE = pressed
									    // in between = previous state

    // Go through and check all 5 buttons for debounce together every ISR
	for (i = 0; i < BUTTON_COUNT; i++) { 
		mask = 1 << i;

        // Handling button pressed here
		if (buttons & mask) { 
			state[i] += BUTTON_STATE_INCREMENT;
			if (state[i] >= BUTTON_PRESSED_STATE) {
				state[i] = BUTTON_PRESSED_STATE;
				_butt.gButtons |= mask; // update debounced button state
			}
		}

        // Handling button released here
		else { 
			state[i] -= BUTTON_STATE_DECREMENT;
			if (state[i] <= 0) {
				state[i] = 0;
				_butt.gButtons &= ~mask;
			}
		}
	}
}

// sample joystick and convert to button presses
void ButtonReadJoystick(void)
{
    ADCProcessorTrigger(ADC0_BASE, 0);          // trigger the ADC sample sequence for Joystick X and Y
    while(!ADCIntStatus(ADC0_BASE, 0, false));  // wait until the sample sequence has completed
    ADCSequenceDataGet(ADC0_BASE, 0, _butt.gJoystick);// retrieve joystick data
    ADCIntClear(ADC0_BASE, 0);                  // clear ADC sequence interrupt flag

    // process joystick movements as button presses using hysteresis
    if (_butt.gJoystick[0] > JOYSTICK_UPPER_PRESS_THRESHOLD) _butt.gButtons |= 1 << 5; // joystick right in position 5
    if (_butt.gJoystick[0] < JOYSTICK_UPPER_RELEASE_THRESHOLD) _butt.gButtons &= ~(1 << 5);

    if (_butt.gJoystick[0] < JOYSTICK_LOWER_PRESS_THRESHOLD) _butt.gButtons |= 1 << 6; // joystick left in position 6
    if (_butt.gJoystick[0] > JOYSTICK_LOWER_RELEASE_THRESHOLD) _butt.gButtons &= ~(1 << 6);

    if (_butt.gJoystick[1] > JOYSTICK_UPPER_PRESS_THRESHOLD) _butt.gButtons |= 1 << 7; // joystick up in position 7
    if (_butt.gJoystick[1] < JOYSTICK_UPPER_RELEASE_THRESHOLD) _butt.gButtons &= ~(1 << 7);

    if (_butt.gJoystick[1] < JOYSTICK_LOWER_PRESS_THRESHOLD) _butt.gButtons |= 1 << 8; // joystick down in position 8
    if (_butt.gJoystick[1] > JOYSTICK_LOWER_RELEASE_THRESHOLD) _butt.gButtons &= ~(1 << 8);
}

// autorepeat button presses if a button is held long enough
uint32_t ButtonAutoRepeat(void)
{
    static int count[BUTTON_AND_JOYSTICK_COUNT] = {0}; // autorepeat counts
    int i;
    uint32_t mask;
    uint32_t presses = 0;
    for (i = 0; i < BUTTON_AND_JOYSTICK_COUNT; i++) {
        mask = 1 << i;
        if (_butt.gButtons & mask)
            count[i]++;     // increment count if button is held
        else
            count[i] = 0;   // reset count if button is let go
        if (count[i] >= BUTTON_AUTOREPEAT_INITIAL &&
                (count[i] - BUTTON_AUTOREPEAT_INITIAL) % BUTTON_AUTOREPEAT_NEXT == 0)
            presses |= mask;    // register a button press due to auto-repeat
    }
    return presses;
}

// Button scanning and debouncing process
uint16_t ButtonGetState(void) {
    // First thing first, clear the interrupt flag so we can exit
//    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Read hardware digital button state
    uint32_t gpio_buttons = (~GPIOPinRead(GPIO_PORTJ_BASE, 0xff) & (GPIO_PIN_1 | GPIO_PIN_0)) // EK-TM4C1294XL buttons in positions 0 and 1
            | ((~GPIOPinRead(GPIO_PORTH_BASE, 0xff) & (GPIO_PIN_1)) << 1) // BoosterPack S1
            | ((~GPIOPinRead(GPIO_PORTK_BASE, 0xff) & (GPIO_PIN_6)) >> 3) // BoosterPack S2
            | ((~GPIOPinRead(GPIO_PORTD_BASE, 0xff) & (GPIO_PIN_4))); // BoosterPack Joystick Select

    // Debouncing and combining the digital and analog signal together into gButtons
    uint32_t old_buttons = _butt.gButtons;    // save previous button state
    ButtonDebounce(gpio_buttons);       // Run the button debouncer. The result is in gButtons.
    ButtonReadJoystick();               // Convert joystick state to button presses. The result is in gButtons.
    
    // Check for button presses here to check repeat behavior
    uint32_t presses = ~old_buttons & _butt.gButtons;   // detect button presses (transitions from not pressed to pressed)
    presses |= ButtonAutoRepeat();      // autorepeat presses if a button is held long enough

    // Store button state into our FIFO queue only if we have changes
    return presses;
}

// Helper function to pop elements from FIFO queue and handle 
void ButtonHandlingTask(void) {
    uint16_t presses = 0;
    while(1) {
        // Pending on the mailbox for user input
        Mailbox_pend(Mailbox_Button, &presses, BIOS_WAIT_FOREVER);

        // Take the semaphore and start handling button pressed here
        Semaphore_pend(sem_accessDisplay, BIOS_WAIT_FOREVER);

        // handling ESR_SW1, change the trigger edge
        if (presses & (1<<4)) _disp.rising = (_disp.rising + 1) & 1;

        // handling pushing the joystick up, increase the voltage scale
        if (presses & (1<<7)) _disp.voltsPerDivPointer = (_disp.voltsPerDivPointer + 1) & ((1<<2)-1);

        // handling pushing the joystick down, decrease the voltage scale
        if (presses & (1<<8)) _disp.voltsPerDivPointer = (_disp.voltsPerDivPointer - 1) & ((1<<2)-1);

        // handling pushing the S2 button, switching mode of operation
        if (presses & (1<<3)) _disp.dispMode = (_disp.dispMode + 1) & 1;

        // We're done, post back to the sempahore
        Semaphore_post(sem_accessDisplay);
    }
}

// Handler for ButtonClock, that will trigger the button scan
void ButtonClockSignal(void) {
    Semaphore_post(sem_ButtonTask);
}

 // Mailbox where the Button Task posts button IDs
void ButtonMailboxTask(void){
    IntMasterEnable();
    uint16_t button_pressed;
    uint16_t last_pressed = 0;
    while(1) {
        // Pending for semaphore, post in Clock signal
        Semaphore_pend(sem_ButtonTask, BIOS_WAIT_FOREVER);

        // Only add new state into the mailbox if changes were made
        button_pressed = ButtonGetState();
        if (button_pressed != last_pressed)
            Mailbox_post(Mailbox_Button, &button_pressed, BIOS_WAIT_FOREVER);
        last_pressed = button_pressed;
    }
}
