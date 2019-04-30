/***************************************************************************//**
 * @file util.c
 * @brief Utility functionality.
 * @version 2.7
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
 *
 *   @todo
 *     - Check the placement of errors and see (if they get send using LoRaWAN) that they don't break clock functionality.
 *     - Go back to INIT state?
 *     - Update used LoRaWAN functionality if necessary.
 *
 * ******************************************************************************
 *
 * @section License
 *
 *   Some methods use code obtained from examples from [Silicon Labs' GitHub](https://github.com/SiliconLabs/peripheral_examples).
 *   These sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


#include <stdint.h>      /* (u)intXX_t */
#include <stdbool.h>     /* "bool", "true", "false" */
#include "em_cmu.h"      /* Clock Management Unit */
#include "em_gpio.h"     /* General Purpose IO */

#include "util.h"        /* Corresponding header file */
#include "pin_mapping.h" /* PORT and PIN definitions */
#include "debugging.h" 	 /* Enable or disable printing to UART */
#include "delay.h"     	 /* Delay functionality */

#ifdef RELEASE /* RELEASE */
#include "lora_wrappers.h" /* LoRaWAN functionality */
#endif /* RELEASE */


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
 *   **DEBUG mode**@n
 *   In *debug mode* this method flashes the LED, displays a UART message
 *   and holds the microcontroller forever in a loop until it gets reset.
 *   The error value gets stored in a global variable.
 *
 *   **RELEASE mode**@n
 *   In *release mode* this method sends the error value to the cloud using
 *   LoRaWAN if the error number doesn't correspond to LoRaWAN-related functionality
 *   (numbers 30 - 50).
 *
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

#ifdef DEBUG /* DEBUG */

#if DEBUGGING == 1 /* DEBUGGING */
	dbcritInt(">>> Error (", number, ")! Please reset MCU. <<<");
#endif /* DEBUGGING */

	while(1)
	{
		delay(100);
		GPIO_PinOutToggle(LED_PORT, LED_PIN); /* Toggle LED */
	}

#else /* RELEASE */

	/* Check if the error number isn't called in LoRaWAN functionality */
	if ((number < 30) && number > 50)
	{
		initLoRaWAN(); /* Initialize LoRaWAN functionality TODO: use something else if we can save the settings */

#if DEBUGGING == 1 /* DEBUGGING */
		dbcritInt(">>> Error (", number, ")! Sending the message to the cloud. <<<");
#endif /* DEBUGGING */

		sendStatus(number); /* Send the status value */

		disableLoRaWAN(); /* Disable RN2483 */
	}
	else
	{

#if DEBUGGING == 1 /* DEBUGGING */
		dbcritInt(">>> Error in LoRaWAN functionality (", number, ")! <<<");
#endif /* DEBUGGING */

	}

#endif /* DEBUG/RELEASE selection */

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

#if DEBUGGING == 1 /* DEBUGGING */
	dbinfo("LED pin initialized");
#endif /* DEBUGGING */

	LED_initialized = true;
}
