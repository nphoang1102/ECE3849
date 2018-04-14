/*
 * RTOS_helper.h
 *
 *  Created on: Apr 5, 2018
 *      Author: hpnguyen
 *
 *  Helper file to import every library required
 *  to run the TI-RTOS
 */

#ifndef RTOS_HELPER_H_
#define RTOS_HELPER_H_

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>

/* Standard C libraries */
#include <stdint.h>
#include <stdbool.h>

/* Interrupt control library */
#include "driverlib/interrupt.h"


#endif /* RTOS_HELPER_H_ */
