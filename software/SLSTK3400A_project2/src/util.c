/***************************************************************************//**
 * @file util.c
 * @brief Utility functions.
 * @version 2.0
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
 *
 ******************************************************************************/


#include "../inc/util.h"


/* Global variables */
uint8_t errorNumber = 0;
bool LEDinitialized = false;


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
	if (!LEDinitialized) initLED();

	/* Set the selected state */
	if (enabled) GPIO_PinOutSet(LED_PORT, LED_PIN);
	else GPIO_PinOutClear(LED_PORT, LED_PIN);
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
	if (!LEDinitialized) initLED();

	/* Save the given number in the global variable */
	errorNumber = number;

#ifdef DEBUGGING /* DEBUGGING */
	dbcritInt(">>> Error (", number, ")! Please reset MCU. <<<");
#endif /* DEBUGGING */

	while(1)
	{
		Delay(100);
		GPIO_PinOutToggle(LED_PORT, LED_PIN); /* Toggle LED */
	}
}


/**************************************************************************//**
 * @brief
 *   Initialize the LED.
 *
 * @note
 *   This method is automatically called by the other methods if necessary.
 *****************************************************************************/
void initLED (void)
{
	/* Enable necessary clock (just in case) */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1".
	 * This means that "GPIO_PinOutClear(...)" is not necessary after this mode change.*/
	GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, 0);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("LED pin initialized");
#endif /* DEBUGGING */

	LEDinitialized = true;
}
