/***************************************************************************//**
 * @file DS18B20.c
 * @brief All code for the DS18B20 temperature sensor.
 * @version 3.1
 * @author
 *   Alec Vanderhaegen & Sarah Goossens@n
 *   Modified by Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Reformatted existing methods to use pin_mapping.h, changed unsigned char to
 *             uint8_t values, added comments and cleaned up includes.
 *   @li v1.1: Added documentation, removed unnecessary GPIO statements regarding
 *             DOUT values of VDD pin.
 *   @li v1.2: Removed some unnecessary GPIO lines and added comments about `out` (DOUT) argument.
 *   @li v1.3: Changed some methods to be static (~hidden).
 *   @li v1.4: Cleaned up includes.
 *   @li v1.5: Made more methods static.
 *   @li v1.6: Updated documentation.
 *   @li v1.7: Started using new delay functionality.
 *   @li v1.8: Added line to disable DATA pin after a measurement, this breaks the code but fixes the sleep current.
 *   @li v1.9: Enabled and disabled timer each time a measurement is taken.
 *   @li v2.0: Updated documentation.
 *   @li v2.1: Changed method to return `uint32_t` instead of float.
 *   @li v2.2: Changed error numbering, moved definition from header to source file and updated header file include.
 *   @li v2.3: Changed *timeout* variable and changed types to `int32_t`.
 *   @li v2.4: Fixed temperature measurement and refined timeout functionality.
 *   @li v2.5: Updated documentation.
 *   @li v2.6: Updated code to don't execute code further if the first initialization failed.
 *   @li v3.0: Disabled initialized functionality before entering an `error` function, added
 *             functionality to exit methods after `error` call and updated version number.
 *   @li v3.1: Removed `static` before the local variable (not necessary).
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
 ******************************************************************************/


#include <stdint.h>        /* (u)intXX_t */
#include <stdbool.h>       /* "bool", "true", "false" */
#include "em_cmu.h"        /* Clock Management Unit */
#include "em_gpio.h"       /* General Purpose IO (GPIO) peripheral API */

#include "DS18B20.h"       /* Corresponding header file */
#include "pin_mapping.h"   /* PORT and PIN definitions */
#include "debug_dbprint.h" /* Enable or disable printing to UART */
#include "delay.h"         /* Delay functionality */
#include "util.h"    	   /* Utility functionality */
#include "ustimer.h"       /* Timer functionality */


/* Local definitions */
/** Enable (1) or disable (0) printing the timeout counter value using DBPRINT */
#define DBPRINT_TIMEOUT 0

/* Maximum values for the counters before exiting a `while` loop */
#define TIMEOUT_INIT       20
#define TIMEOUT_CONVERSION 500 /* 12 bit resolution (reset default) = 750 ms max resolving time */


/* Local variable */
bool DS18B20_VDD_initialized = false;


/* Local prototypes */
static void powerDS18B20 (bool enabled);
static bool init_DS18B20 (void);
static void writeByteToDS18B20 (uint8_t data);
static uint8_t readByteFromDS18B20 (void);
static int32_t convertTempData (uint8_t tempLS, uint8_t tempMS);


/**************************************************************************//**
 * @brief
 *   Get a temperature value from the DS18B20.
 *
 * @details
 *   USTimer gets initialized, the sensor gets powered, the data-transmission
 *   takes place, the timer gets de-initialized to disable the clocks and interrupts,
 *   the data and power pin get disabled and finally the read values are converted
 *   to an `int32_t` value.@n
 *   **Negative temperatures work fine.**
 *
 * @return
 *   The read temperature data.
 *****************************************************************************/
int32_t readTempDS18B20 (void)
{
	/* Timeout counter */
	uint16_t counter = 0;

	/* Variable to indicate if a conversion has been completed */
	bool conversionCompleted = false;

	/* Variable to hold raw data bytes */
	uint8_t rawDataFromDS18B20Arr[9] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	/* Initialize timer
	 * Initializing and disabling the timer again adds about 40 µs active time but should conserve sleep energy... */
	USTIMER_Init();

	/* Initialize and power VDD pin */
	powerDS18B20(true);

	/* Power-up delay of 5 ms */
	delay(5);

	/* Initialize communication and only continue if successful */
	if (init_DS18B20())
	{
		writeByteToDS18B20(0xCC); /* 0xCC = "Skip Rom" (address all devices on the bus simultaneously without sending out any ROM code information) */
		writeByteToDS18B20(0x44); /* 0x44 = "Convert T" */

		/* MASTER now generates "read time slots", the DS18B20 will write HIGH to the bus if the conversion is completed
		 *   The datasheet gives the following directions for time slots, but reading bytes also seems to work...
		 *     - Read time slots have a 60 µs duration and 1 µs recovery between slots
		 *     - After the master pulls the line low for 1 µs, the data is valid for up to 15 µs */
		while ((counter < TIMEOUT_CONVERSION) && !conversionCompleted)
		{
			uint8_t testByte = readByteFromDS18B20();
			if (testByte > 0) conversionCompleted = true;

			counter++;
		}

		/* Exit the function if the maximum waiting time was reached */
		if (counter == TIMEOUT_CONVERSION)
		{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
			dbcrit("Waiting time for DS18B20 conversion reached!");
#endif /* DEBUG_DBPRINT */

			/* Disable interrupts and turn off the clock to the underlying hardware timer. */
			USTIMER_DeInit();

			/* Disable data pin (otherwise we got a "sleep" current of about 330 µA due to the on-board 10k pull-up) */
			GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeDisabled, 0);

			/* Disable the VDD pin */
			powerDS18B20(false);

			error(29);

			/* Exit function */
			return (0);

		}
#if DBPRINT_TIMEOUT == 1 /* DBPRINT_TIMEOUT */
		else
		{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
			dbwarnInt("DS18B20 conversion (", counter, ")");
#endif /* DEBUG_DBPRINT */

		}
#endif /* DBPRINT_TIMEOUT */

		init_DS18B20();           /* Initialize communication */
		writeByteToDS18B20(0xCC); /* 0xCC = "Skip Rom" */
		writeByteToDS18B20(0xBE); /* 0xCC = "Read Scratchpad" */

		/* Read the bytes */
		for (uint8_t i = 0; i < 9; i++) rawDataFromDS18B20Arr[i] = readByteFromDS18B20();

		/* Disable interrupts and turn off the clock to the underlying hardware timer. */
		USTIMER_DeInit();

		/* Disable data pin (otherwise we got a "sleep" current of about 330 µA due to the on-board 10k pull-up) */
		GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeDisabled, 0);

		/* Disable the VDD pin */
		powerDS18B20(false);

		/* Return the converted byte */
		return (convertTempData(rawDataFromDS18B20Arr[0], rawDataFromDS18B20Arr[1]));
	}
	else
	{
		/* Disable interrupts and turn off the clock to the underlying hardware timer. */
		USTIMER_DeInit();

		/* Disable data pin (otherwise we got a "sleep" current of about 330 µA due to the on-board 10k pull-up) */
		GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeDisabled, 0);

		/* Disable the VDD pin */
		powerDS18B20(false);

		/* Exit function */
		return (0);
	}
}


/**************************************************************************//**
 * @brief
 *   Enable or disable the power to the temperature sensor.
 *
 * @details
 *   This method also initializes the pin-mode if necessary.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] enabled
 *   @li `true` - Enable the GPIO pin connected to the VDD pin of the temperature sensor.
 *   @li `talse` - Disable the GPIO pin connected to the VDD pin of the temperature sensor.
 *****************************************************************************/
static void powerDS18B20 (bool enabled)
{
	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Initialize VDD pin if not already the case */
	if (!DS18B20_VDD_initialized)
	{
		/* In the case of gpioModePushPull", the last argument directly sets the pin state */
		GPIO_PinModeSet(TEMP_VDD_PORT, TEMP_VDD_PIN, gpioModePushPull, enabled);

		DS18B20_VDD_initialized = true;
	}
	else
	{
		if (enabled) GPIO_PinOutSet(TEMP_VDD_PORT, TEMP_VDD_PIN); /* Enable VDD pin */
		else GPIO_PinOutClear(TEMP_VDD_PORT, TEMP_VDD_PIN); /* Disable VDD pin */
	}
}


/**************************************************************************//**
 * @brief
 *   Initialize communication to the DS18B20.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @return
 *   @li `true` - *Presence* pulse detected in time.
 *   @li `false` - No *presence* pulse detected.
 *****************************************************************************/
static bool init_DS18B20 (void)
{
	/* Timeout counter */
	uint32_t counter = 0;

	/* MASTER RESET: Pull data line LOW for at least 480 µs (Master TX) */
	GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModePushPull, 0); /* gpioModePushPull: Last argument directly sets the pin state */
	USTIMER_DelayIntSafe(480);

	/* Change pin-mode to input - External pull-up resistor pulls data line back HIGH */
	GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInput, 0);

	/* Check if the line becomes LOW (~ wait while it stays high) during the maximum waiting time
	 *   The DS18B20 should detect the data line rising due to the pull-up resistor, waits 15 - 50 µs
	 *    and then pulls the line back LOW (for 60 - 240 µs) to indicate it's PRESENCE */
	while ((counter < TIMEOUT_INIT) && (GPIO_PinInGet(TEMP_DATA_PORT, TEMP_DATA_PIN) == 1)) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_INIT)
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("No DS18B20 presence pulse detected in time!");
#endif /* DEBUG_DBPRINT */

		error(28);

		return (false);
	}
#if DBPRINT_TIMEOUT == 1 /* DBPRINT_TIMEOUT */
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbwarnInt("DS18B20 INIT (", counter, ")");
#endif /* DEBUG_DBPRINT */

	}
#endif /* DBPRINT_TIMEOUT */

	/* Master RX should be at least 480 µs */
	USTIMER_DelayIntSafe(480);

	return (true);
}


/**************************************************************************//**
 * @brief
 *   Write a byte (`uint8_t`) to the DS18B20.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] data
 *   The data to write to the DS18B20.
 *****************************************************************************/
static void writeByteToDS18B20 (uint8_t data)
{
	/* In the case of gpioModePushPull", the last argument directly sets the pin state */
	GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModePushPull, 0);

	/* Write the byte, bit by bit */
	for (uint8_t i = 0; i < 8; i++)
	{
		/* Check if we need to write a "1" */
		if (data & 0x01)
		{
			GPIO_PinOutClear(TEMP_DATA_PORT, TEMP_DATA_PIN);

			/* 5 µs delay should be called here but this loop works fine too... */
			for (uint8_t i=0; i<5; i++);

			GPIO_PinOutSet(TEMP_DATA_PORT, TEMP_DATA_PIN);
			USTIMER_DelayIntSafe(60);
		}
		/* If not, write a "0" */
		else
		{
			GPIO_PinOutClear(TEMP_DATA_PORT, TEMP_DATA_PIN);
			USTIMER_DelayIntSafe(60);
			GPIO_PinOutSet(TEMP_DATA_PORT, TEMP_DATA_PIN);

			/* 5 µs delay should be called here but this loop works fine too... */
			for (uint8_t i=0; i<5; i++);
		}
		/* Right shift bits once */
		data >>= 1;
	}

	/* Set data line high */
	GPIO_PinOutSet(TEMP_DATA_PORT, TEMP_DATA_PIN);
}


/**************************************************************************//**
 * @brief
 *   Read a byte (`uint8_t`) from the DS18B20.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @return
 *   The byte read from the DS18B20.
 *****************************************************************************/
static uint8_t readByteFromDS18B20 (void)
{
	/* Data to eventually return */
	uint8_t data = 0x0;

	/* Read the byte, bit by bit */
	for (uint8_t i = 0; i < 8; i++)
	{
		/* Change pin-mode to input */
		GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInput, 0);

		/* Right shift bits once */
		data >>= 1;

		/* If the line is high, OR the first bit of the data:
		 * 0x80 = 1000 0000 */
		if (GPIO_PinInGet(TEMP_DATA_PORT, TEMP_DATA_PIN)) data |= 0x80;

		/* In the case of gpioModePushPull", the last argument directly sets the pin state */
		GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModePushPull, 1);

		/* Wait some time before going into next loop */
		USTIMER_DelayIntSafe(70);
	}
	return (data);
}


/**************************************************************************//**
 * @brief
 *   Convert temperature data.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] tempLS
 *   Least significant byte.
 *
 * @param[in] tempMS
 *   Most significant byte.
 *
 * @return
 *   The converted temperature data.
 *****************************************************************************/
static int32_t convertTempData (uint8_t tempLS, uint8_t tempMS)
{
	uint16_t rawDataMerge;
	uint16_t reverseRawDataMerge;

	int32_t finalTemperature;

	/* Check if it is a negative temperature value
	 * 0xF8 = 0b1111 1000 */
	if (tempMS & 0xF8)
	{
		rawDataMerge = tempMS;

		/* Left shift 8 times */
		rawDataMerge <<= 8;

		/* Add the second part */
		rawDataMerge += tempLS;

		/* Invert the value since we have a negative temperature */
		reverseRawDataMerge = ~rawDataMerge;

		/* Calculate the final temperature */
		finalTemperature = -(reverseRawDataMerge + 1) * 62.5;
	}
	/* We're dealing with a positive temperature */
	else
	{
		rawDataMerge = tempMS;

		/* Left shift 8 times */
		rawDataMerge <<= 8;

		/* Add the second part */
		rawDataMerge += tempLS;

		/* Calculate the final temperature */
		finalTemperature = rawDataMerge * 62.5;
	}

	return (finalTemperature);
}
