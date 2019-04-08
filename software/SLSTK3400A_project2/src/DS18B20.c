/***************************************************************************//**
 * @file DS18B20.c
 * @brief All code for the DS18B20 temperature sensor.
 * @version 1.6
 * @author
 *   Alec Vanderhaegen & Sarah Goossens
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
 *
 *   @todo RTC sleep functionality is broken when `UDELAY_Calibrate()` is called.
 *           - UDelay uses RTCC, Use timers instead! (timer + prescaler: every microsecond an interrupt?)
 *         Use internal pull-up resistor for DATA pin using DOUT argument.
 *           - Not working, why? `GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInputPull, 1);`
 *         Enter EM1 when the MCU is waiting in a delay method? (see `readVBAT` method in `other.c`)
 *
 ******************************************************************************/


#include <stdint.h>             /* (u)intXX_t */
#include <stdbool.h>            /* "bool", "true", "false" */
#include "em_cmu.h"             /* Clock Management Unit */
#include "em_gpio.h"            /* General Purpose IO (GPIO) peripheral API */

#include "../inc/udelay.h"      /* Microsecond delay routine TODO: Use something else */

#include "../inc/DS18B20.h"     /* Corresponding header file */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h"   /* Enable or disable printing to UART */
#include "../inc/util.h"    	/* Utility functionality */


/* Local variable */
static bool DS18B20_VDD_initialized = false;


/* Local prototypes */
static void powerDS18B20 (bool enabled);
static bool init_DS18B20 (void);
static void writeByteToDS18B20 (uint8_t data);
static uint8_t readByteFromDS18B20 (void);
static float convertTempData (uint8_t tempLS, uint8_t tempMS);


/**************************************************************************//**
 * @brief
 *   Get a temperature value from the DS18B20.
 *
 * @details
 *   One measurement takes about 550 ms.
 *
 * @return
 *   The read temperature data (`float`).
 *****************************************************************************/
float readTempDS18B20 (void)
{
	/* Initialize and power VDD pin */
	powerDS18B20(true);

	/* Raw data bytes */
	uint8_t rawDataFromDS18B20Arr[9] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	/* Read a temperature value (see datasheet) if initialization was successful */
	if (init_DS18B20())
	{
		writeByteToDS18B20(0xCC); /* 0xCC = "Skip Rom" */
		writeByteToDS18B20(0x44); /* 0x44 = "Convert T" */

		init_DS18B20();

		writeByteToDS18B20(0xCC); /* 0xCC = "Skip Rom" */
		writeByteToDS18B20(0xBE); /* 0xCC = "Read Scratchpad" */

		/* Read the bytes */
		for (uint8_t n = 0; n < 9; n++)
		{
			rawDataFromDS18B20Arr[n] = readByteFromDS18B20();
		}

		/* Disable the VDD pin */
		powerDS18B20(false);

		/* Return the converted byte */
		return (convertTempData(rawDataFromDS18B20Arr[0], rawDataFromDS18B20Arr[1]));
	}
	else
	{
#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("DS18B20 measurement failed");
#endif /* DEBUGGING */

		error(11);

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
 *   Initialize (reset) DS18B20.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @return
 *   @li `true` - Initialization (reset) successful.
 *   @li `false` - Initialization (reset) failed.
 *****************************************************************************/
static bool init_DS18B20 (void)
{
	uint16_t counter = 0;

	/* In the case of gpioModePushPull", the last argument directly sets the pin state */
	GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModePushPull, 0);
	UDELAY_Delay(480);

	/* Change pin-mode to input */
	GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInput, 0); /* TODO: Try to use internal pullup? */

	/* Check if the line becomes HIGH during the maximum waiting time */
	while (counter++ <= MAX_TIME_CTR && GPIO_PinInGet(TEMP_DATA_PORT, TEMP_DATA_PIN) == 1)
	{
		/* TODO: Maybe EMU_EnterEM1() */
	}

	/* Exit the function if the maximum waiting time was reached (reset failed) */
	if (counter == MAX_TIME_CTR)
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("DS18B20 initialization failed");
#endif /* DEBUGGING */

		return (false);
	}

	/* Reset counter value */
	counter = 0;

	/* Check if the line becomes LOW during the maximum waiting time */
	while (counter++ <= MAX_TIME_CTR && GPIO_PinInGet(TEMP_DATA_PORT, TEMP_DATA_PIN) == 0)
	{
		/* TODO: Maybe EMU_EnterEM1() */
	}

	/* Exit the function if the maximum waiting time was reached (reset failed) */
	if (counter == MAX_TIME_CTR)
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("DS18B20 initialization failed");
#endif /* DEBUGGING */

		return (false);
	}

	/* Continue waiting and finally return that the reset was successful */
	UDELAY_Delay(480);

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
			UDELAY_Delay(5);
			GPIO_PinOutSet(TEMP_DATA_PORT, TEMP_DATA_PIN);
			UDELAY_Delay(60);
		}
		/* If not, write a "0" */
		else
		{
			GPIO_PinOutClear(TEMP_DATA_PORT, TEMP_DATA_PIN);
			UDELAY_Delay(60);
			GPIO_PinOutSet(TEMP_DATA_PORT, TEMP_DATA_PIN);
			UDELAY_Delay(5);
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
		GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInput, 0); /* TODO: Try to use internal pullup? */

		/* Right shift bits once */
		data >>= 1;

		/* If the line is high, OR the first bit of the data:
		 * 0x80 = 1000 0000 */
		if (GPIO_PinInGet(TEMP_DATA_PORT, TEMP_DATA_PIN)) data |= 0x80;

		/* In the case of gpioModePushPull", the last argument directly sets the pin state */
		GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModePushPull, 1);


		/* Wait some time before going into next loop */
		UDELAY_Delay(70);
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
 *   Most significant byte
 *
 * @return
 *   The converted temperature data (`float`).
 *****************************************************************************/
static float convertTempData (uint8_t tempLS, uint8_t tempMS)
{
	uint16_t rawDataMerge;
	uint16_t reverseRawDataMerge;
	float finalTemperature;

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
		finalTemperature = -(reverseRawDataMerge + 1) * 0.0625;
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
		finalTemperature = rawDataMerge * 0.0625;
	}

	return (finalTemperature);
}
