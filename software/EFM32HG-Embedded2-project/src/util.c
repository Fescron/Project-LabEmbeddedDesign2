/***************************************************************************//**
 * @file util.c
 * @brief Utility functionality.
 * @version 3.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Started with the code from https://github.com/Fescron/Project-LabEmbeddedDesign1/tree/master/code/SLSTK3400A_ADXL362
 *   @li v1.1: Changed PinModeSet DOUT value to 0 in initLED.
 *   @li v1.2: Removed unnecessary `GPIO_PinOutClear` line in initLED.
 *   @li v1.3: Moved initRTCcomp method from `main.c` to here, added delay functionality which goes into EM1 or EM2.
 *   @li v1.4: Moved delay functionality to specific header and source files.
 *   @li v2.0: Changed `Error()` to `error()`, added a global variable to keep the error number and initialize the pin of the LED automatically.
 *   @li v2.1: Changed initLED to be a static (~hidden) method and also made the global variables static.
 *   @li v2.2: Added peripheral clock enable/disable functionality for energy saving purposes, only added necessary includes in header file,
 *             moved the others to the source file, updated documentation, replaced SysTick delay with RTCC delay, changed error delay length.
 *   @li v2.3: Changed name of static variable, simplified some logic.
 *   @li v2.4: Stopped disabling the GPIO clock.
 *   @li v2.5: Moved documentation.
 *   @li v2.6: Updated code with new DEFINE checks.
 *   @li v2.7: Added functionality to send error values using LoRaWAN.
 *   @li v2.8: Added the ability to enable/disable error forwarding to the cloud using a public definition and changed UART error color.
 *   @li v3.0: Updated version number.
 *
 * ******************************************************************************
 *
 * @todo
 *   **Future improvements:**@n
 *     - Only send a maximum amount of errors to the could using LoRaWAN according to a defined value.
 *         - Reset the counter in the MEASURE/SEND state?
 *     - Go back to INIT state on an error call?
 *         - `GOTO` is supported in C but is dangerous to use (nested loops, ...)
 *         - Check if the clock functionality doesn't break when this is implemented ...
 *
 * ******************************************************************************
 *
 * @section License
 *
 *   **Copyright (C) 2019 - Brecht Van Eeckhoudt**
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the **GNU General Public License** as published by
 *   the Free Software Foundation, either **version 3** of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   *A copy of the GNU General Public License can be found in the `LICENSE`
 *   file along with this source code.*
 *
 *   @n
 *
 *   Some methods use code obtained from examples from [Silicon Labs' GitHub](https://github.com/SiliconLabs/peripheral_examples).
 *   These sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


#include <stdint.h>        /* (u)intXX_t */
#include <stdbool.h>       /* "bool", "true", "false" */
#include "em_cmu.h"        /* Clock Management Unit */
#include "em_gpio.h"       /* General Purpose IO */

#include "util.h"          /* Corresponding header file */
#include "pin_mapping.h"   /* PORT and PIN definitions */
#include "debug_dbprint.h" /* Enable or disable printing to UART */
#include "delay.h"         /* Delay functionality */

#if ERROR_FORWARDING == 1 /* ERROR_FORWARDING */
#include "lora_wrappers.h" /* LoRaWAN functionality */
#endif /* ERROR_FORWARDING */


/* Local variables */
static uint8_t errorNumber = 0;
static bool LED_initialized = false;


/* Local prototype */
static void initLED (void);


/**************************************************************************//**
 * @brief
 *   Enable or disable the LED.
 *
 * @details
 *   This method also initializes the pin-mode if necessary.
 *
 * @param[in] enabled
 *   @li `true` - Enable LED.
 *   @li `false` - Disable LED.
 *****************************************************************************/
void led (bool enabled)
{
	/* Initialize LED if not already the case */
	if (!LED_initialized) initLED();

	/* Set the selected state */
	if (enabled) GPIO_PinOutSet(LED_PORT, LED_PIN);
	else GPIO_PinOutClear(LED_PORT, LED_PIN);
}


/**************************************************************************//**
 * @brief
 *   Error method.
 *
 * @details
 *   **ERROR_FORWARDING == 0**@n
 *   The method flashes the LED, displays a UART message and holds the MCU
 *   forever in a loop until it gets reset. The error value gets stored in
 *   a global variable.
 *
 *   **ERROR_FORWARDING == 1**@n
 *   The method sends the error value to the cloud using LoRaWAN if the error
 *   number doesn't correspond to LoRaWAN-related functionality (numbers 30 - 55).
 *
 * @param[in] number
 *   The number to indicate where in the code the error was thrown.
 *****************************************************************************/
void error (uint8_t number)
{
	/* Initialize LED if not already the case */
	if (!LED_initialized) initLED();

	/* Save the given number in the global variable */
	errorNumber = number;

#if ERROR_FORWARDING == 0 /* ERROR_FORWARDING */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
	dbprint_color(">>> Error (", 5);
	dbprintInt(number);
	dbprintln_color(")! Please reset MCU. <<<", 5);
#endif /* DEBUG_DBPRINT */

	while (1)
	{
		delay(100);
		GPIO_PinOutToggle(LED_PORT, LED_PIN); /* Toggle LED */
	}

#else /* ERROR_FORWARDING */

	/* Check if the error number isn't called in LoRaWAN functionality */
	if ((number < 30) || (number > 55))
	{
#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbprint_color(">>> Error (", 5);
		dbprintInt(number);
		dbprintln_color(")! Sending the message to the cloud. <<<", 5);
#endif /* DEBUG_DBPRINT */

		initLoRaWAN(); /* Initialize LoRaWAN functionality */

		sendStatus(number); /* Send the status value */

		disableLoRaWAN(); /* Disable RN2483 */
	}
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbprint_color(">>> Error in LoRaWAN functionality (", 5);
		dbprintInt(number);
		dbprintln_color(")! <<<", 5);
#endif /* DEBUG_DBPRINT */

	}

#endif /* ERROR_FORWARDING */

}


/**************************************************************************//**
 * @brief
 *   Initialize the LED.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *****************************************************************************/
static void initLED (void)
{
	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* In the case of gpioModePushPull, the last argument directly sets the pin state */
	GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, 0);

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
	dbinfo("LED pin initialized");
#endif /* DEBUG_DBPRINT */

	LED_initialized = true;
}
