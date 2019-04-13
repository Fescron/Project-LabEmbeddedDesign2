/***************************************************************************//**
 * @file cable.c
 * @brief Cable checking functionality.
 * @version 1.3
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Move `checkCable` from `main.c` to this file and add battery voltage measurement logic.
 *   @li v1.1: Updated code with new DEFINE checks.
 *   @li v1.2: Moved ADC functionality to `adc.c`.
 *   @li v1.3: Changed filename to `cable.c`
 *
 *   @todo
 *     - Fix cable-checking method.
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
#include "em_device.h"   /* Include necessary MCU-specific header file */
#include "em_cmu.h"      /* Clock management unit */
#include "em_gpio.h"     /* General Purpose IO */

#include "cable.h"       /* Corresponding header file */
#include "pin_mapping.h" /* PORT and PIN definitions */
#include "debugging.h"   /* Enable or disable printing to UART for debugging */
#include "delay.h"     	 /* Delay functionality */


/**************************************************************************//**
 * @brief
 *   Method to check if the wire is broken.
 *
 * @details
 *   This method sets the mode of the pins, checks the connection
 *   between them and also disables them at the end.
 *
 * @return
 *   @li `true` - The connection is still okay.
 *   @li `false` - The connection is broken!
 *****************************************************************************/
bool checkCable (void)
{
	/* TODO: Fix this method */

	/* Value to eventually return */
	bool check = false;

	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Change mode of first pin */
	GPIO_PinModeSet(BREAK1_PORT, BREAK1_PIN, gpioModeInput, 1); /* TODO: "1" = filter enabled? */

	/* Change mode of second pin and also set it high with the last argument */
	GPIO_PinModeSet(BREAK2_PORT, BREAK2_PIN, gpioModePushPull, 1);

	delay(50);

	/* Check the connection */
	if (!GPIO_PinInGet(BREAK1_PORT,BREAK1_PIN)) check = true;

	/* Disable the pins */
	GPIO_PinModeSet(BREAK1_PORT, BREAK1_PIN, gpioModeDisabled, 0);
	GPIO_PinModeSet(BREAK2_PORT, BREAK2_PIN, gpioModeDisabled, 0);

#if DEBUGGING == 1 /* DEBUGGING */
	if (check) dbinfo("Cable still intact");
	else dbcrit("Cable broken!");
#endif /* DEBUGGING */

	return (check);
}
