/*
 * pwm_gen.c
 *
 *  Created on: Apr 22, 2018
 *      Author: hpnguyen
 *
 *  Source code for the function generator
 */

// Standard C libraries
#include <stdint.h>
#include <stdbool.h>

// Driver libraries from TivaWare
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/tm4c1294ncpdt.h"

// Local header files from project
#include "pwm_gen.h"
#include "pwm_table.h"

// Initialize struct space for global variable
struct PWM _pwm = {
    0, // phase accumulator
    (1<<23)*23, // phase increment, equal to 2^32/2^10*(465/20)
};

// Import global variable
extern uint8_t gPWMWaveformTable[PWM_WAVEFORM_TABLE_SIZE];

// PWM initialization
void pwm_init(void) {
    // use M0PWM1, at GPIO PF1, which is BoosterPack Connector #1 pin 40
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1); // PF1 = M0PWM1
    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);

    // configure the PWM0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);             // use system clock
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PWM_PERIOD);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD/2); // initial 50% duty cycle
    PWMOutputInvert(PWM0_BASE, PWM_OUT_1_BIT, true);      // invert PWM output
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);       // enable PWM output
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);                   // enable PWM generator

    // enable PWM interrupt in the PWM peripheral
    PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
}

// PWM function generator ISR
void pwm_ISR(void) {
    // clear PWM interrupt flag
    PWMGenIntClear(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);

    _pwm.gPhase += _pwm.gPhaseIncrement;

    // write directly to the Compare B register that determines the duty cycle
    PWM0_0_CMPB_R = 1 + gPWMWaveformTable[_pwm.gPhase >> (32 - PWM_WAVEFORM_INDEX_BITS)];

}
