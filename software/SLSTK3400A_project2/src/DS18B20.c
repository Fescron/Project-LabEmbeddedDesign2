/***************************************************************************//**
 * @file DS18B20.c
 * @brief All code for the DS18B20 temperature sensor.
 * @version 1.4
 * @author
 *   Alec Vanderhaegen & Sarah Goossens
 *   Modified by Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   v1.0: Reformatted existing methods to use pin_mapping.h, changed unsigned char to
 *         uint8_t values, added comments and cleaned up includes.
 *   v1.1: Added documentation, removed unnecessary GPIO statements regarding
 *         DOUT values of VDD pin.
 *   v1.2: Removed some unnecessary GPIO lines and added comments about "out" (DOUT) argument.
 *   v1.3: Changed some methods to be static (~hidden).
 *   v1.4: Cleaned up includes.
 *
 *   TODO: Remove stdint and stdbool includes?
 *         Use internal pull-up resistor for DATA pin using DOUT argument.
 *           => Not working, why? GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInputPull, 1);
 *         Enter EM1 when the MCU is waiting in a delay method?
 *
 ******************************************************************************/


/* Includes necessary for this source file */
//#include <stdint.h>             /* (u)intXX_t */
//#include <stdbool.h>            /* "bool", "true", "false" */
#include "em_cmu.h"             /* Clock Management Unit */
#include "em_gpio.h"            /* General Purpose IO (GPIO) peripheral API */
#include "../inc/udelay.h"      /* Microsecond delay routine */

#include "../inc/DS18B20.h"     /* Corresponding header file */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h"   /* Enable or disable printing to UART */


/* Prototypes for static methods only used by other methods in this file
 * (Not available to be used elsewhere) */
static void writeByteToDS18B20 (uint8_t data);
static uint8_t readByteFromDS18B20 (void);
static float convertTempData (uint8_t tempLS, uint8_t tempMS);


/**************************************************************************//**
 * @brief
 *   Get a temperature value from the DS18B20.
 *
 * @details
 *   One measurement takes 550 ms.
 *
 * @return
 *   The read temperature data (float). An impossible value is returned
 *   if the initialization (reset) failed.
 *****************************************************************************/
float readTempDS18B20 (void)
{
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

		/* Return the converted byte */
		return (convertTempData(rawDataFromDS18B20Arr[0], rawDataFromDS18B20Arr[1]));
	}
	else
	{
#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("DS18B20 measurement failed");
#endif /* DEBUGGING */

		/* Return an impossible value if the measurement failed */
		return (10000);
	}
}


/**************************************************************************//**
 * @brief
 *   Initialize the VDD pin for the DS18B20.
 *****************************************************************************/
void initVDD_DS18B20 (void)
{
	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1". */
	GPIO_PinModeSet(TEMP_VDD_PORT, TEMP_VDD_PIN, gpioModePushPull, 1);

}


/**************************************************************************//**
 * @brief
 *   Enable or disable the power to the temperature sensor.
 *
 * @param[in] enabled
 *   @li True - Enable the GPIO pin connected to the VDD pin of the temperature sensor.
 *   @li False - Disable the GPIO pin connected to the VDD pin of the temperature sensor.
 *****************************************************************************/
void powerDS18B20 (bool enabled)
{
	if (enabled) GPIO_PinOutSet(TEMP_VDD_PORT, TEMP_VDD_PIN);
	else GPIO_PinOutClear(TEMP_VDD_PORT, TEMP_VDD_PIN);
}


/**************************************************************************//**
 * @brief
 *   Initialize (reset) DS18B20.
 *
 * @return
 *   @li true - Initialization (reset) successful.
 *   @li false - Initialization (reset) failed.
 *****************************************************************************/
bool init_DS18B20 (void)
{
	uint16_t counter = 0;

	/* In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1". */
	GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModePushPull, 0);
	UDELAY_Delay(480);

	/* Change pin-mode to input */
	GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInput, 0); /* TODO: Try to use internal pullup */

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
 *   Write a byte (uint8_t) to the DS18B20.
 *
 * @note
 *   This is a static (~hidden) method because it's only internally used
 *   in this file and called by other methods if necessary.
 *
 * @param[in] data
 *   The data to write to the DS18B20.
 *****************************************************************************/
static void writeByteToDS18B20 (uint8_t data)
{
	/* In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1". */
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
 *   Read a byte (uint8_t) from the DS18B20.
 *
 * @note
 *   This is a static (~hidden) method because it's only internally used
 *   in this file and called by other methods if necessary.
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
		GPIO_PinModeSet(TEMP_DATA_PORT, TEMP_DATA_PIN, gpioModeInput, 0); /* TODO: Try to use internal pullup */

		/* Right shift bits once */
		data >>= 1;

		/* If the line is high, OR the first bit of the data:
		 * 0x80 = 1000 0000 */
		if (GPIO_PinInGet(TEMP_DATA_PORT, TEMP_DATA_PIN)) data |= 0x80;

		/* In the case of gpioModePushPull", the last argument directly sets the
		 * the pin low if the value is "0" or high if the value is "1". */
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
 *   This is a static (~hidden) method because it's only internally used
 *   in this file and called by other methods if necessary.
 *
 * @param[in] tempLS
 *   Least significant byte.
 *
 * @param[in] tempMS
 *   Most significant byte
 *
 * @return
 *   The converted temperature data (float).
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
