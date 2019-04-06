/***************************************************************************//**
 * @file util.c
 * @brief Utility functions.
 * @version 2.2
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   v1.0: Started with the code from https://github.com/Fescron/Project-LabEmbeddedDesign1/tree/master/code/SLSTK3400A_ADXL362
 *   v1.1: Changed PinModeSet DOUT value to 0 in initLED.
 *   v1.2: Removed unnecessary "GPIO_PinOutClear" line in initLED.
 *   v1.3: Moved initRTCcomp method from "main.c" to here, added delay functionality which goes into EM1 or EM2.
 *   v1.4: Moved delay functionality to specific header and source files.
 *   v2.0: Changed "Error()" to "error()", added a global variable to keep the error number and initialize the pin of the LED automatically.
 *   v2.1: Changed initLED to be a static (~hidden) method and also made the global variables static.
 *   v2.2: Added peripheral clock enable/disable functionality for energy saving purposes, only added necessary includes in header file,
 *         moved the others to the source file, updated documentation, replaced SysTick delay with RTCC delay, changed error delay length.
 *
 *   TODO: Also enable/disable "cmuClock_HFPER"?
 *         Remove stdint and stdbool includes?
 *
 ******************************************************************************/


/* Includes necessary for this source file */
//#include <stdint.h>    /* (u)intXX_t */
//#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_cmu.h"    /* Clock Management Unit */
#include "em_gpio.h"   /* General Purpose IO */

#include "../inc/util.h"        /* Corresponding header file */
#include "../inc/delay.h"     	/* Delay functionality */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h" 	/* Enable or disable printing to UART */


/* Static variables only available and used in this file */
static uint8_t errorNumber = 0;
static bool LED_init = false;


/* Prototype for static method only used by other methods in this file
 * (Not available to be used elsewhere) */
static void initLED (void);


/**************************************************************************//**
 * @brief
 *   Enable or disable the LED.
 *
 * @details
 *   This method also initializes the pin-mode if necessary.
 *
 * @param[in] enabled
 *   @li True - Enable LED
 *   @li False - Disable LED.
 *****************************************************************************/
void led (bool enabled)
{
	/* Initialize LED if not already the case */
	if (!LED_init)
	{
		initLED();
	}
	else
	{
		/* Enable necessary clock */
		CMU_ClockEnable(cmuClock_GPIO, true);
	}

	/* Set the selected state */
	if (enabled) GPIO_PinOutSet(LED_PORT, LED_PIN);
	else GPIO_PinOutClear(LED_PORT, LED_PIN);

	/* Disable used clock */
	CMU_ClockEnable(cmuClock_GPIO, false);
}


/**************************************************************************//**
 * @brief
 *   Error method.
 *
 * @details
 *   Flashes the LED, displays a UART message and holds
 *   the microcontroller forever in a loop until it gets reset. Also
 *   stores the error number in a global variable.
 *
 * @param[in] number
 *   The number to indicate where in the code the error was thrown.
 *****************************************************************************/
void error (uint8_t number)
{
	/* Initialize LED if not already the case */
	if (!LED_init)
	{
		initLED();
	}
	else
	{
		/* Enable necessary clock */
		CMU_ClockEnable(cmuClock_GPIO, true);
	}

	/* Save the given number in the global variable */
	errorNumber = number;

#ifdef DEBUGGING /* DEBUGGING */
	dbcritInt(">>> Error (", number, ")! Please reset MCU. <<<");
#endif /* DEBUGGING */

	while(1)
	{
		delay(100);
		GPIO_PinOutToggle(LED_PORT, LED_PIN); /* Toggle LED */
	}
}


/**************************************************************************//**
 * @brief
 *   Initialize the LED.
 *
 * @note
 *   This is a static (~hidden) method because it's only internally used
 *   in this file and called by other methods if necessary.
 *****************************************************************************/
static void initLED (void)
{
	/* Enable necessary clock */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1".
	 * This means that "GPIO_PinOutClear(...)" is not necessary after this mode change.*/
	GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, 0);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("LED pin initialized");
#endif /* DEBUGGING */

	LED_init = true;
}