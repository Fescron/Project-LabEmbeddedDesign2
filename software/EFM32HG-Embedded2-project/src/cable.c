/***************************************************************************//**
 * @file cable.c
 * @brief Cable checking functionality.
 * @version 1.5
 * @author
 *   Matthias Alleman@n
 *   Modified by Brecht Van Eeckhoudt
 * @details
 *   Originally this functionality should've detected a cable breaking event using
 *   an **interrupt**, but due to the low water resistance this unfortunately wasn't
 *   possible to implement.
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Move `checkCable` from `main.c` to this file and add battery voltage measurement logic.
 *   @li v1.1: Updated code with new DEFINE checks.
 *   @li v1.2: Moved ADC functionality to `adc.c`.
 *   @li v1.3: Changed filename to `cable.c`
 *   @li v1.4: Fixed cable checking method.
 *   @li v1.5: Moved more functionality to this file.
 *
 *   @todo
 *     - Also send measurements on cable breaking?
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


#include <stdint.h>        /* (u)intXX_t */
#include <stdbool.h>       /* "bool", "true", "false" */
#include "em_device.h"     /* Include necessary MCU-specific header file */
#include "em_cmu.h"        /* Clock management unit */
#include "em_gpio.h"       /* General Purpose IO */

#include "cable.h"         /* Corresponding header file */
#include "pin_mapping.h"   /* PORT and PIN definitions */
#include "debug_dbprint.h" /* Enable or disable printing to UART for debugging */
#include "lora_wrappers.h" /* LoRaWAN functionality */

/* Local variable */
/** Keep the amount of times a message has been send to indicate the cable is broken (only send 4 messages in total) */
uint8_t cableBrokenSendTimes = 0;

/* Local prototype */
static bool checkCable_internal (void);


/**************************************************************************//**
 * @brief
 *   Method to check if the wire is broken.
 *****************************************************************************/
void checkCable (void)
{
	/* Check if the cable is broken */
	if (!checkCable_internal())
	{
		/* Only send a message 4 times */
		if (cableBrokenSendTimes < 4)
		{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
			dbcrit("Cable broken! Sending the data ...");
#endif /* DEBUG_DBPRINT */

			initLoRaWAN(); /* Initialize LoRaWAN functionality */

			sendCableBroken(true); /* Send the LoRaWAN message TODO: also send measurement data here? */

			disableLoRaWAN(); /* Disable RN2483 */

			cableBrokenSendTimes++; /* Increment the counter */
		}
		else
		{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
			dbcrit("Cable broken but no longer sending the data");
#endif /* DEBUG_DBPRINT */

		}
	}
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbinfo("Cable still intact");
#endif /* DEBUG_DBPRINT */

	}
}


/**************************************************************************//**
 * @brief
 *   Method to check if the wire is broken.
 *
 * @details
 *   This method enables the necessary GPIO clocks, sets the mode of the pins,
 *   checks the connection between them and also disables them at the end.
 *
 * @return
 *   @li `true` - The connection is still okay.
 *   @li `false` - The connection is broken!
 *****************************************************************************/
static bool checkCable_internal (void)
{
	/* Value to eventually return */
	bool check = false;

	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Change mode of first pin */
	GPIO_PinModeSet(BREAK1_PORT, BREAK1_PIN, gpioModeInput, 0);

	/* Change mode of second pin and also set it low with the last argument */
	GPIO_PinModeSet(BREAK2_PORT, BREAK2_PIN, gpioModePushPull, 0);

	/* Check the connection */
	if (!GPIO_PinInGet(BREAK1_PORT,BREAK1_PIN)) check = true;

	/* Disable the pins */
	GPIO_PinModeSet(BREAK1_PORT, BREAK1_PIN, gpioModeDisabled, 0);
	GPIO_PinModeSet(BREAK2_PORT, BREAK2_PIN, gpioModeDisabled, 0);

	return (check);
}
