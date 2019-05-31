/***************************************************************************//**
 * @file main.c
 * @brief The main file for Project 2 from Embedded System Design 2 - Lab.
 * @version 5.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   Please check https://github.com/Fescron/Project-LabEmbeddedDesign2/tree/master/software to find the latest version!
 *
 *   @li v1.0: Started from https://github.com/Fescron/Project-LabEmbeddedDesign1
 *             and added code for the DS18B20 temperature sensor and the selfmade
 *             link breakage sensor. Reformatted some of these imported methods.
 *   @li v1.1: Removed unused files, add cable-checking method.
 *   @li v1.2: Moved initRTCcomp method to "util.c".
 *   @li v1.3: Stopped using deprecated function "GPIO_IntConfig".
 *   @li v1.4: Started using get/set method for the static variable "ADXL_triggered".
 *   @li v1.5: Reworked the code a lot (stopped disabling cmuClock_GPIO, ...).
 *   @li v1.6: Moved all documentation above source files to this file.
 *   @li v1.7: Updated documentation, started using a state-machine.
 *   @li v1.8: Moved checkCable method to "other.c" and started using readVBAT method.
 *   @li v1.9: Cleaned up documentation and TODO's.
 *   @li v2.0: Updated code with new DEFINE checks.
 *   @li v2.1: Updated code with new ADC functionality.
 *   @li v2.2: Started using working temperature sensor code.
 *   @li v2.3: Started using working cable checking code and added "SEND" `mcu_state`.
 *   @li v2.4: Started using a struct to keep the measurements in, added line to disable the RN2483.
 *   @li v2.5: Started using custom enum types for the accelerometer settings.
 *   @li v2.6: Added ADXL testing functionality.
 *   @li v2.7: Removed USTIMER logic from this file.
 *   @li v2.8: Added functionality to detect a *storm*.
 *   @li v2.9: Started adding LoRaWAN functionality, added more wake-up/sleep functionality.
 *   @li v3.0: Started using updated LoRaWAN send methods.
 *   @li v3.1: Added functionality to enable/disable the LED blinking when measuring/sending data.
 *   @li v3.2: Fixed button wake-up when sleeping for WAKE_UP_PERIOD_S/2.
 *   @li v3.3: Moved `data.index` reset to LoRaWAN sending functionality and updated documentation.
 *   @li v3.4: Moved `data.index` reset back to `main.c`.
 *   @li v3.5: Added functionality to go back to the remaining sleep time on an accelerometer wake-up.
 *   @li v3.6: Updated code to handle underlying changes.
 *   @li v4.0: Updated documentation and version number.
 *   @li v4.1: Added definitions to easily change the accelerometer's configuration settings.
 *   @li v5.0: Updated sleep logic when waking up using the accelerometer.
 *
 * ******************************************************************************
 *
 * @todo
 *   **Optimalisations:**@n
 *     - Change LoRaWAN Spreading Factor and send power, and check the effect on the power usage.
 *     - Add some time to the `while` increment escape counters?
 *     - Check power usage effect of disabling `PM_SENS_EXT`.
 *     - Check difference between absolute/referenced mode for the accelerometer.
 *     - Change *mode* to release.
 *
 * @todo
 *   **Future improvements:**@n
 *     - Use *IAR toolchain* instead of *GCC*, see section **Optimizing code** in `documentation.h`
 *     - Sleep for some time in *while* loops instead of just incrementing the *escape-counter*.
 *         - First separate `ULFRCO` definition in `delay.c
 *     - Try to shorten delays (for example: 40 ms power-up time for the RN2483...)
 *     - Fix `sleepLoRaWAN` and `wakeLoRaWAN` functionality (also see `lora_wrappers.c`).
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
 *   Some methods also use code obtained from examples from [Silicon Labs' GitHub](https://github.com/SiliconLabs/peripheral_examples).
 *   These sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 * ******************************************************************************
 *
 * @attention
 *   See the file `documentation.h` for a lot of useful documentation about this project!
 *
 * ******************************************************************************
 *
 * @section Errors
 *
 *   If in this project something unexpected occurs, an `error` method gets called.
 *   What happens in this method can be selected in `util.h` with the definition
 *   `ERROR_FORWARDING`. If it's value is `0` the MCU displays (if `dbprint` is enabled)
 *   a UART message and gets put in a `while(true)` to flash the LED. If it's value is
 *   `1` then certain values (all values except 30 - 55 since these are errors in the
 *   LoRaWAN functionality itself) get forwarded to the cloud using LoRaWAN functionality
 *   and the MCU resumes it's code.
 *
 *   When calling an `error` method, the following things were kept in mind:
 *     - **Values 0 - 9:** Reserved for reset and other critical functionality.
 *         - **1:** Send once after a reset (`firstBoot == true` in `main.c`)
 *         - **9:** Used in `sendTest(data);` method (`lora_wrappers.c`)
 *     - **Values 10 - ... :** Available to use for anything else.
 *
 *   Below is a list of values which correspond to certain functionality:
 *     - **10:** Problem in the case of the state machine (`main.c`)
 *     - **11 - 13:** `adc.c`
 *     - **14 - 17:** `delay.c`
 *     - **18 - 19:** `interrupt.c`
 *     - **20 - 27:** `ADXL362.c`
 *     - **28 - 29:** `DS18B20.c`
 *     - **30 - 50:** `lora_wrappers.c`
 *     - **51 - 55:** `leuart.c`
 *
 * ******************************************************************************
 *
 * @section CHANNELS LoRaWAN sensor channels
 *
 *   - `LPP_VBAT_CHANNEL            0x10 // 16`
 *   - `LPP_TEMPERATURE_CHANNEL_INT 0x11 // 17`
 *   - `LPP_TEMPERATURE_CHANNEL_EXT 0x12 // 18`
 *   - `LPP_STORM_CHANNEL           0x13 // 19`
 *   - `LPP_CABLE_BROKEN_CHANNEL    0x14 // 20`
 *   - `LPP_STATUS_CHANNEL          0x15 // 21`
 *
 ******************************************************************************/


#include <stdint.h>        /* (u)intXX_t */
#include <stdbool.h>       /* "bool", "true", "false" */
#include "em_device.h"     /* Include necessary MCU-specific header file */
#include "em_chip.h"       /* Chip Initialization */
#include "em_cmu.h"        /* Clock management unit */
#include "em_gpio.h"       /* General Purpose IO */
#include "em_rtc.h"        /* Real Time Counter (RTC) */

#include "pin_mapping.h"   /* PORT and PIN definitions */
#include "debug_dbprint.h" /* Enable or disable printing to UART for debugging */
#include "delay.h"         /* Delay functionality */
#include "util.h"          /* Utility functionality */
#include "interrupt.h"     /* GPIO wake-up initialization and interrupt handlers */
#include "ADXL362.h"       /* Functions related to the accelerometer */
#include "DS18B20.h"       /* Functions related to the temperature sensor */
#include "adc.h"           /* Internal voltage and temperature reading functionality */
#include "cable.h"         /* Cable checking functionality */
#include "lora_wrappers.h" /* LoRaWAN functionality */
#include "datatypes.h"     /* Definitions of the custom data-types */


/* Local definitions */
/** Time between each wake-up in seconds
 *    @li max 500 seconds when using LFXO delay
 *    @li 3600 seconds (one hour) works fine when using ULFRCO delay */
#define WAKE_UP_PERIOD_S   1800 /* 600 = every 10 minutes */

/** Amount of PIN interrupt wakeups (before a RTC wake-up) to be considered as a *storm* */
#define STORM_INTERRUPTS   8

/** The threshold value [g] for the accelerometer to detect and send an interrupt to wake-up the MCU */
#define ADXL_THRESHOLD     7

/** The *g* range to configure the accelerometer with */
#define ADXL_RANGE         ADXL_RANGE_8G

/** The ODR setting to configure the accelerometer with */
#define ADXL_ODR           ADXL_ODR_12_5_HZ

/** Public definition to select if the LED is turned on while measuring or sending data
 *    @li `1` - Enable the LED when while measuring or sending data.
 *    @li `0` - Don't enable the LED while measuring or sending data. */
#define LED_ENABLED        0


/* Local variables */
/** Keep the state of the state machine */
static MCU_State_t MCUstate;

/** Keep the measurement data */
static MeasurementData_t data;


/**************************************************************************//**
 * @brief
 *   Method to check if any interrupts are triggered by buttons.
 *
 * @return
 *   @li `true` - A button interrupt was triggered.
 *   @li `false` - No button interrupt was triggered.
 *****************************************************************************/
bool checkBTNinterrupts (void)
{
	bool value = false;

	if (BTN_getTriggered(0))
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbprintln_color("PB0 pushed!", 4);
#endif /* DEBUG_DBPRINT */

		value = true;

		BTN_setTriggered(0, false); /* Clear static variable */
	}


	if (BTN_getTriggered(1))
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbprintln_color("PB1 pushed!", 4);
#endif /* DEBUG_DBPRINT */

		value = true;

		BTN_setTriggered(1, false); /* Clear static variable */
	}

	return (value);
}


/**************************************************************************//**
 * @brief
 *   Main function.
 *****************************************************************************/
int main (void)
{
	/* Keep a value if a storm has been detected */
	bool stormDetected = false;

	/* Value used to send one test LoRaWAN message after booting */
	bool firstBoot = true;

	/* Value to keep the remaining sleep time when waking up using the accelerometer */
	uint32_t remainingSleeptime = 0;

	/* Set the index to put the measurements in */
	data.index = 0;

	while (1)
	{
		switch (MCUstate)
		{
			case INIT:
			{
				CHIP_Init(); /* Initialize chip */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
#if CUSTOM_BOARD == 1 /* Custom Happy Gecko pinout */
				dbprint_INIT(DBG_UART, DBG_UART_LOC, false, false);
				dbwarn("CUSTOM board pinout selected");
#else /* Regular Happy Gecko pinout */
				dbprint_INIT(USART1, 4, true, false); /* VCOM */
				dbwarn("REGULAR board pinout selected");
#endif /* Board pinout selection */
#endif  /* DEBUG_DBPRINT */

				led(true); /* Enable (and initialize) LED */

				delay(4000); /* 4 second delay to notice initialization */
				led(false);
				delay(100);
				led(true);

				initGPIOwakeup(); /* Initialize GPIO wake-up */

				initADC(BATTERY_VOLTAGE); /* Initialize ADC to read battery voltage */

				/* Initialize pin and disable power to RN2483 */
				GPIO_PinModeSet(PM_RN2483_PORT, PM_RN2483_PIN, gpioModePushPull, 0);

				/* Initialize pin and disable external sensor power on DRAMCO shield */
				GPIO_PinModeSet(PM_SENS_EXT_PORT, PM_SENS_EXT_PIN, gpioModePushPull, 0); // TODO: check power usage effect?

				/* Initialize accelerometer */
				if (true)
				{
					/* Go through all of the accelerometer ODR settings to see the influence they have on power usage. */
					if (false)
					{
						initADXL(); /* Initialize the accelerometer */

						led(false);
						testADXL();
						led(true);
					}

					initADXL(); /* Initialize the accelerometer */

					ADXL_configRange(ADXL_RANGE); /* Set the measurement range */

					ADXL_configODR(ADXL_ODR); /* Configure ODR */

					if (false) ADXL_readValues(); /* Read and display values forever */

					ADXL_configActivity(ADXL_THRESHOLD); /* Configure (referenced) activity threshold mode on INT1 [g] */

					ADXL_enableMeasure(true); /* Enable measurements */

					delay(300);

					ADXL_ackInterrupt(); /* ADXL gives interrupt, capture this and acknowledge it by reading from it's status register */

					ADXL_enableSPI(false); /* Disable SPI after the initializations */

					ADXL_clearCounter(); /* Clear the trigger counter */
				}

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
				dbprintln("");
#endif /* DEBUG_DBPRINT */

				led(false); /* Disable LED */

				MCUstate = MEASURE;
			} break;

			case MEASURE:
			{

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(true); /* Enable LED */
#endif /* LED_ENABLED */

				/* Measure and store the external temperature */
				data.extTemp[data.index] = readTempDS18B20();

				/* Measure and store the battery voltage */
				data.voltage[data.index] = readADC(BATTERY_VOLTAGE);

				/* Measure and store the internal temperature */
				data.intTemp[data.index] = readADC(INTERNAL_TEMPERATURE);

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
				dbinfoInt("Measurement ", data.index + 1, "");
				dbinfoInt("Temperature: ", data.extTemp[data.index], "");
				dbinfoInt("Battery voltage: ", data.voltage[data.index], "");
				dbinfoInt("Internal temperature: ", data.intTemp[data.index], "");
#endif /* DEBUG_DBPRINT */

				/* Increase the index to put the next measurements in */
				data.index++;

				/* Check if the cable is intact and send LoRaWAN data if necessary */
				if (checkCable(data)) data.index = 0; /* If measurements have been send, reset the index */

				if (firstBoot && (data.index > 0))
				{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
					dbwarn("Sending first measurements ...");
#endif /* DEBUG_DBPRINT */

					initLoRaWAN(); /* Initialize LoRaWAN functionality */

					sendStatus(1); /* Send the status data */

					sendMeasurements(data); /* Send the measurements */

					disableLoRaWAN(); /* Disable RN2483 */

					data.index = 0; /* Reset the index to put the measurements in (needs to be here for the correct data to be affected) */

					firstBoot = false;
				}

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(false); /* Disable LED */
#endif /* LED_ENABLED */

				/* Decide if 6 measurements are taken or not and react accordingly */
				if (data.index == 6) MCUstate = SEND;
				else MCUstate = SLEEP;
			} break;

			case SEND:
			{

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(true); /* Enable LED */
#endif /* LED_ENABLED */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
				dbwarnInt("Sending ", data.index, " measurements ...");
#endif /* DEBUG_DBPRINT */

				initLoRaWAN(); /* Initialize LoRaWAN functionality */

				sendMeasurements(data); /* Send the measurements */

				disableLoRaWAN(); /* Disable RN2483 */

				data.index = 0; /* Reset the index to put the measurements in (needs to be here for the correct data to be affected) */

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(false); /* Disable LED */
#endif /* LED_ENABLED */

				MCUstate = SLEEP;
			} break;

			case SEND_STORM:
			{

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(true); /* Enable LED */
#endif /* LED_ENABLED */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
				dbwarnInt("STORM DETECTED! Sending ", data.index, " measurement(s) ...");
#endif /* DEBUG_DBPRINT */

				initLoRaWAN(); /* Initialize LoRaWAN functionality */

				sendStormDetected(true); /* Send a message to indicate that a storm has been detected */

				if (data.index > 0)
				{
					sendMeasurements(data); /* Send the already gathered measurements */

					data.index = 0; /* Reset the index to put the measurements in (needs to be here for the correct data to be affected) */
				}

				disableLoRaWAN(); /* Disable RN2483 */

				stormDetected = true; /* Indicate that a storm has been detected */

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(false); /* Disable LED */
#endif /* LED_ENABLED */

				MCUstate = SLEEP_HALFTIME;
			} break;

			case SLEEP:
			{
				sleep(WAKE_UP_PERIOD_S); /* Go to sleep for xx seconds */

				MCUstate = WAKEUP;
			} break;

			case SLEEP_HALFTIME:
			{
				sleep(WAKE_UP_PERIOD_S/2); /* Go to sleep for xx seconds */

				MCUstate = WAKEUP;
			} break;

			case WAKEUP:
			{

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(true); /* Enable LED */
#endif /* LED_ENABLED */

				/* Check if we woke up using buttons */
				if (checkBTNinterrupts())
				{
					ADXL_clearCounter(); /* Clear the trigger counter */
					remainingSleeptime = 0; /* Reset passed sleeping time */

					MCUstate = MEASURE; /* Take measurements on "case WAKEUP" exit */
				}

				/* Check if we woke up using the RTC sleep functionality and act accordingly */
				if (RTC_checkWakeup())
				{
					RTC_clearWakeup(); /* Clear static variable */

					ADXL_clearCounter(); /* Clear the trigger counter because we woke up "normally" */
					remainingSleeptime = 0; /* Reset passed sleeping time since it's an RTC wakeup */

					MCUstate = MEASURE; /* Take measurements on "case WAKEUP" exit */
				}

				/* Check if we woke up using the accelerometer */
				if (ADXL_getTriggered())
				{
					/* Check if we detected a storm */
					if (ADXL_getCounter() > STORM_INTERRUPTS)
					{
						RTC_Enable(false); /* Disable the counter */

						MCUstate = SEND_STORM; /* Storm detected, send a message on "case WAKEUP" exit */
					}
					else
					{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
						dbprintln_color("INT-PD7 triggered!", 4);
#endif /* DEBUG_DBPRINT */

						ADXL_enableSPI(true);  /* Enable SPI functionality */
						ADXL_ackInterrupt();   /* Acknowledge ADXL interrupt by reading the status register */
						ADXL_enableSPI(false); /* Disable SPI functionality */

						/* Check if a storm was detected before going to sleep for WAKE_UP_PERIOD_S/2 */
						if (stormDetected)
						{
							stormDetected = false; /* Reset variable */
							MCUstate = MEASURE; /* Take measurements on "case WAKEUP" exit */
						}
						else
						{
							remainingSleeptime += RTC_getPassedSleeptime(); /* Add the time spend sleeping to the variable */

#if LED_ENABLED == 1 /* LED_ENABLED */
							led(false); /* Disable LED */
#endif /* LED_ENABLED */

							/* Go back to sleep for the remaining time until another measurement (we only take measurements on RTC/button wake-up) */
							sleep(WAKE_UP_PERIOD_S - remainingSleeptime);

							MCUstate = WAKEUP; /* Go back to this case if we wake-up */
						}
					}
				}

#if LED_ENABLED == 1 /* LED_ENABLED */
				led(false); /* Disable LED */
#endif /* LED_ENABLED */

			} break;

			default:
			{
				error(10);
			} break;
		}
	}
}
