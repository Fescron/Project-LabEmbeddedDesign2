/***************************************************************************//**
 * @file lpp.c
 * @brief Basic Low Power Payload (LPP) functionality.
 * @version 1.3
 * @author
 *   Geoffrey Ottoy@n
 *   Modified by Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: DRAMCO GitHub version (https://github.com/DRAMCO/EFM32-RN2483-LoRa-Node).
 *   @li v1.1: Added custom methods to add specific data to the LPP packet.
 *   @li v1.2: Merged methods to use less bytes when sending the data.
 *   @li v1.3: Updated documentation.
 *
 ******************************************************************************/

/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: lpp.c
 *      Created: 2018-03-23
 *       Author: Geoffrey Ottoy - Modified by Brecht Van Eeckhoudt
 *
 *  Description: Basic Low Power Payload (LPP) functionality.
 */

#include <em_device.h> /* Math functionality */
#include <stdlib.h>    /* Memory functionality */
#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */

#include "lpp.h"       /* Corresponding header file */
#include "datatypes.h" /* Definitions of the custom data-types */

/* LPP types */
#define LPP_DIGITAL_INPUT			0x00
#define LPP_ANALOG_INPUT			0x02
#define LPP_TEMPERATURE				0x67
#define LPP_HUMIDITY				0x68
#define LPP_ACCELEROMETER			0x71
#define LPP_PRESSURE				0x73

/* LPP data sizes */
#define LPP_DIGITAL_INPUT_SIZE		0x03
#define LPP_ANALOG_INPUT_SIZE		0x04
#define LPP_TEMPERATURE_SIZE		0x04
#define LPP_HUMIDITY_SIZE			0x03
#define LPP_ACCELEROMETER_SIZE		0x08
#define LPP_PRESSURE_SIZE			0x04

/* LPP channel ID's */
#define LPP_DIGITAL_INPUT_CHANNEL	0x01
#define LPP_ANALOG_INPUT_CHANNEL	0x02
#define LPP_TEMPERATURE_CHANNEL		0x03
#define LPP_HUMIDITY_CHANNEL		0x04
#define LPP_ACCELEROMETER_CHANNEL	0x05
#define LPP_PRESSURE_CHANNEL		0x06

/* Custom channel ID's */
#define LPP_VBAT_CHANNEL            0x10 /* 16 */
#define LPP_TEMPERATURE_CHANNEL_INT 0x11 /* 17 */
#define LPP_TEMPERATURE_CHANNEL_EXT 0x12 /* 18 */
#define LPP_STORM_CHANNEL           0x13 /* 19 */
#define LPP_CABLE_BROKEN_CHANNEL    0x14 /* 20 */
#define LPP_STATUS_CHANNEL          0x15 /* 21 */

bool LPP_InitBuffer(LPP_Buffer_t *b, uint8_t size)
{
	LPP_ClearBuffer(b);

	b->buffer = (uint8_t *) malloc(sizeof(uint8_t) * size);

	if(b->buffer != NULL)
	{
		b->fill = 0;
		b->length = size;
		return (true);
	}

	return (false);
}

void LPP_ClearBuffer(LPP_Buffer_t *b)
{
	if(b->buffer != NULL) free(b->buffer);
}

/**************************************************************************//**
 * @brief
 *   Add measurement data to the LPP packet following the *custom message
 *   convention* to save bytes to send.
 *
 * @details
 *   This is what each added byte represents, **in the case of `2` measurements**:
 *     - **byte 0:** Amount of measurements
 *     - **byte 1:** Battery voltage channel (`LPP_VBAT_CHANNEL = 0x10`)
 *     - **byte 2:** LPP analog input type (`LPP_ANALOG_INPUT = 0x02`)
 *     - **byte 3-4:** A battery voltage measurement
 *     - **byte 5-6:** Another battery voltage measurement (in the case of `2` measurements)
 *     - ...
 *     - **byte 7:** Internal temperature channel (`LPP_TEMPERATURE_CHANNEL_INT = 0x11`)
 *     - **byte 8:** LPP temperature type (`LPP_TEMPERATURE = 0x67`)
 *     - **byte 9-10:** An internal temperature measurement
 *     - **byte 11-12:** Another internal temperature measurement (in the case of `2` measurements)
 *     - ...
 *     - **byte 13:** External temperature channel (`LPP_TEMPERATURE_CHANNEL_EXT = 0x12`)
 *     - **byte 14:** LPP temperature type (`LPP_TEMPERATURE = 0x67`)
 *     - **byte 15-16:** An external temperature measurement
 *     - **byte 17-18:** Another external temperature measurement (in the case of `2` measurements)
 *     - ...
 *
 *   If we have **6 measurements** we need **43 bytes**:
 *     - `1 byte` to hold the amount of measurements
 *     - `2 bytes` to hold the battery voltage channel and LPP analog input type
 *     - `6*2bytes` to hold the battery voltage measurements
 *     - `2 bytes` to hold the internal temperature channel and LPP temperature type
 *     - `6*2bytes` to hold the internal temperature measurements
 *     - `2 bytes` to hold the external temperature channel and LPP temperature type
 *     - `6*2bytes` to hold the external temperature measurements
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] data
 *   The struct which contains the measurements to send using LoRaWAN.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_AddMeasurements (LPP_Buffer_t *b, MeasurementData_t data)
{
	/* Calculate free space in the buffer */
	uint8_t space = b->length - b->fill;

	/* Calculate necessary space, first byte holds the amount of measurements */
	uint8_t necessarySpace = 1;

	/* Add space necessary for the battery voltage and internal and external temperature measurements.
	 * (1 byte for the channel ID, 1 byte for the data type, 2 bytes for each measurement) */
	necessarySpace += 3*(2+(2*(data.index)));

	/* Return `false` if we don't have the necessary space available */
	if (space < necessarySpace) return (false);

	/* Fill the first byte with the amount of measurements */
	b->buffer[b->fill++] = data.index;

	/* Fill the next bytes with battery voltage measurements */
	b->buffer[b->fill++] = LPP_VBAT_CHANNEL;
	b->buffer[b->fill++] = LPP_ANALOG_INPUT;

	for (uint8_t i = 0; i < data.index; i++)
	{
		/* Convert battery voltage value (should represent 0.01 signed ) */
		int16_t batteryLPP = (int16_t)(round((float)data.voltage[i]/10));

		b->buffer[b->fill++] = (uint8_t)((0xFF00 & batteryLPP) >> 8);
		b->buffer[b->fill++] = (uint8_t)(0x00FF & batteryLPP);
	}

	/* Fill the next bytes with internal temperature measurements */
	b->buffer[b->fill++] = LPP_TEMPERATURE_CHANNEL_INT;
	b->buffer[b->fill++] = LPP_TEMPERATURE;

	for (uint8_t i = 0; i < data.index; i++)
	{
		/* Convert temperature sensor value (should represent 0.1 °C Signed MSB) */
		int16_t intTempLPP = (int16_t)(round((float)data.intTemp[i]/100));

		b->buffer[b->fill++] = (uint8_t)((0xFF00 & intTempLPP) >> 8);
		b->buffer[b->fill++] = (uint8_t)(0x00FF & intTempLPP);
	}

	/* Fill the next bytes with external temperature measurements */
	b->buffer[b->fill++] = LPP_TEMPERATURE_CHANNEL_EXT;
	b->buffer[b->fill++] = LPP_TEMPERATURE;

	for (uint8_t i = 0; i < data.index; i++)
	{
		/* Convert temperature sensor value (should represent 0.1 °C Signed MSB) */
		int16_t extTempLPP = (int16_t)(round((float)data.extTemp[i]/100));

		b->buffer[b->fill++] = (uint8_t)((0xFF00 & extTempLPP) >> 8);
		b->buffer[b->fill++] = (uint8_t)(0x00FF & extTempLPP);
	}

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add a value to indicate that a storm has been detected to the LPP packet
 *   following the *custom message convention*.
 *
 * @details
 *   This is what each added byte represents:
 *     - **byte 0:** Amount of measurements (in this case always one)
 *     - **byte 1:** *Storm detected* channel (`LPP_STORM_CHANNEL = 0x13`)
 *     - **byte 2:** LPP digital input type (`LPP_DIGITAL_INPUT = 0x00`)
 *     - **byte 3:** The `stormDetected` value
 *
 *   **We always need 4 bytes.**
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] stormDetected
 *   If a storm is detected using the accelerometer or not.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_AddStormDetected (LPP_Buffer_t *b, uint8_t stormDetected)
{
	/* Calculate free space in the buffer */
	uint8_t space = b->length - b->fill;

	/* Return `false` if we don't have the necessary space available */
	if (space < LPP_DIGITAL_INPUT_SIZE + 1) return (false); /* "+1": One extra byte for the amount of measurements */

	/* Fill the first byte with the amount of measurements (in this case always one) */
	b->buffer[b->fill++] = 0x01;

	/* Fill the next bytes following the default LPP packet convention */
	b->buffer[b->fill++] = LPP_STORM_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = stormDetected;

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add a value to indicate that the cable has been broken to the LPP packet
 *   following the *custom message convention*.
 *
 * @details
 *   This is what each added byte represents:
 *     - **byte 0:** Amount of measurements (in this case always one)
 *     - **byte 1:** *Cable broken* channel (`LPP_CABLE_BROKEN_CHANNEL = 0x14`)
 *     - **byte 2:** LPP digital input type (`LPP_DIGITAL_INPUT = 0x00`)
 *     - **byte 3:** The `cableBroken` value
 *
 *   **We always need 4 bytes.**
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] cableBroken
 *   The *cable break* value.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_AddCableBroken (LPP_Buffer_t *b, uint8_t cableBroken)
{
	/* Calculate free space in the buffer */
	uint8_t space = b->length - b->fill;

	/* Return `false` if we don't have the necessary space available */
	if (space < LPP_DIGITAL_INPUT_SIZE + 1) return (false); /* "+1": One extra byte for the amount of measurements */

	/* Fill the first byte with the amount of measurements (in this case always one) */
	b->buffer[b->fill++] = 0x01;

	/* Fill the next bytes following the default LPP packet convention */
	b->buffer[b->fill++] = LPP_CABLE_BROKEN_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = cableBroken;

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add a value to indicate a program status to the LPP packet following the
 *   *custom message convention*.
 *
 * @details
 *   This is what each added byte represents:
 *     - **byte 0:** Amount of measurements (in this case always one)
 *     - **byte 1:** *Status* channel (`LPP_STATUS_CHANNEL = 0x15`)
 *     - **byte 2:** LPP digital input type (`LPP_DIGITAL_INPUT = 0x00`)
 *     - **byte 3:** The `status` value
 *
 *   **We always need 4 bytes.**
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] status
 *   The *status* value.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_AddStatus (LPP_Buffer_t *b, uint8_t status)
{
	/* Calculate free space in the buffer */
	uint8_t space = b->length - b->fill;

	/* Return `false` if we don't have the necessary space available */
	if (space < LPP_DIGITAL_INPUT_SIZE + 1) return (false); /* "+1": One extra byte for the amount of measurements */

	/* Fill the first byte with the amount of measurements (in this case always one) */
	b->buffer[b->fill++] = 0x01;

	/* Fill the next bytes following the default LPP packet convention */
	b->buffer[b->fill++] = LPP_STATUS_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = status;

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add a battery voltage measurement to the LPP packet, disguised as an
 *   *Analog Input* packet (2 bytes). The channel is defined by `LPP_VBAT_CHANNEL`
 *   and is `0x10`.
 *
 * @deprecated
 *   This is a deprecated method following the standard LPP format to add data
 *   to the LPP packet. In practice it uses too much bytes to send a lot of
 *   measurements at once. The method is kept here just in case.
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] vbat
 *   The measured battery voltage.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_deprecated_AddVBAT (LPP_Buffer_t *b, int16_t vbat)
{
	uint8_t space = b->length - b->fill;
	if (space < LPP_ANALOG_INPUT_SIZE) return (false);

	b->buffer[b->fill++] = LPP_VBAT_CHANNEL;
	b->buffer[b->fill++] = LPP_ANALOG_INPUT;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & vbat) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & vbat);

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add an internal temperature measurement (2 bytes) to the LPP packet.
 *   The channel is defined by `LPP_TEMPERATURE_CHANNEL_INT` and is `0x11`.
 *
 * @deprecated
 *   This is a deprecated method following the standard LPP format to add data
 *   to the LPP packet. In practice it uses too much bytes to send a lot of
 *   measurements at once. The method is kept here just in case.
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] intTemp
 *   The measured internal temperature.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_deprecated_AddIntTemp (LPP_Buffer_t *b, int16_t intTemp)
{
	uint8_t space = b->length - b->fill;
	if (space < LPP_TEMPERATURE_SIZE) return (false);

	b->buffer[b->fill++] = LPP_TEMPERATURE_CHANNEL_INT;
	b->buffer[b->fill++] = LPP_TEMPERATURE;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & intTemp) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & intTemp);

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add an external temperature measurement (2 bytes) to the LPP packet.
 *   The channel is defined by `LPP_TEMPERATURE_CHANNEL_EXT` and is `0x12`.
 *
 * @deprecated
 *   This is a deprecated method following the standard LPP format to add data
 *   to the LPP packet. In practice it uses too much bytes to send a lot of
 *   measurements at once. The method is kept here just in case.
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] extTemp
 *   The measured external temperature.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_deprecated_AddExtTemp (LPP_Buffer_t *b, int16_t extTemp)
{
	uint8_t space = b->length - b->fill;
	if (space < LPP_TEMPERATURE_SIZE) return (false);

	b->buffer[b->fill++] = LPP_TEMPERATURE_CHANNEL_EXT;
	b->buffer[b->fill++] = LPP_TEMPERATURE;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & extTemp) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & extTemp);

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add a *storm* value to the LPP packet, disguised as a *Digital Input*
 *   packet (1 byte). The channel is defined by `LPP_STORM_CHANNEL` and is `0x13`.
 *
 * @deprecated
 *   This is a deprecated method following the standard LPP format to add data
 *   to the LPP packet. The method is kept here just in case.
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] stormDetected
 *   If a storm is detected using the accelerometer or not.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_deprecated_AddStormDetected (LPP_Buffer_t *b, uint8_t stormDetected)
{
	uint8_t space = b->length - b->fill;
	if (space < LPP_DIGITAL_INPUT_SIZE) return (false);

	b->buffer[b->fill++] = LPP_STORM_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = stormDetected;

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add a *cable break* value to the LPP packet, disguised as a *Digital Input*
 *   packet (1 byte). The channel is defined by `LPP_CABLE_BROKEN_CHANNEL` and is `0x14`.
 *
 * @deprecated
 *   This is a deprecated method following the standard LPP format to add data
 *   to the LPP packet. The method is kept here just in case.
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] cableBroken
 *   The *cable break* value.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_deprecated_AddCableBroken (LPP_Buffer_t *b, uint8_t cableBroken)
{
	uint8_t space = b->length - b->fill;
	if (space < LPP_DIGITAL_INPUT_SIZE) return (false);

	b->buffer[b->fill++] = LPP_CABLE_BROKEN_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = cableBroken;

	return (true);
}

/**************************************************************************//**
 * @brief
 *   Add a *status* value to the LPP packet, disguised as a *Digital Input*
 *   packet (1 byte). The channel is defined by `LPP_STATUS_CHANNEL` and is `0x15`.
 *
 * @deprecated
 *   This is a deprecated method following the standard LPP format to add data
 *   to the LPP packet. The method is kept here just in case.
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] status
 *   The *status* value.
 *
 * @return
 *   @li `true` - Successfully added the data to the LoRaWAN packet.
 *   @li `false` - Couldn't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_deprecated_AddStatus (LPP_Buffer_t *b, uint8_t status)
{
	uint8_t space = b->length - b->fill;
	if (space < LPP_DIGITAL_INPUT_SIZE) return (false);

	b->buffer[b->fill++] = LPP_STATUS_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = status;

	return (true);
}

bool LPP_AddDigital(LPP_Buffer_t *b, uint8_t data)
{
	uint8_t space = b->length - b->fill;
	if(space < LPP_DIGITAL_INPUT_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_DIGITAL_INPUT_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = data;

	return (true);
}

bool LPP_AddAnalog(LPP_Buffer_t *b, int16_t data)
{
	uint8_t space = b->length - b->fill;
	if(space < LPP_ANALOG_INPUT_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_ANALOG_INPUT_CHANNEL;
	b->buffer[b->fill++] = LPP_ANALOG_INPUT;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & data) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & data);

	return (true);
}

bool LPP_AddTemperature(LPP_Buffer_t *b, int16_t data)
{
	uint8_t space = b->length - b->fill;
	if(space < LPP_TEMPERATURE_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_TEMPERATURE_CHANNEL;
	b->buffer[b->fill++] = LPP_TEMPERATURE;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & data) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & data);

	return (true);
}

bool LPP_AddHumidity(LPP_Buffer_t *b, uint8_t data)
{
	uint8_t space = b->length - b->fill;
	if(space < LPP_HUMIDITY_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_HUMIDITY_CHANNEL;
	b->buffer[b->fill++] = LPP_HUMIDITY;
	b->buffer[b->fill++] = data;

	return (true);
}

bool LPP_AddAccelerometer(LPP_Buffer_t *b, int16_t x, int16_t y, int16_t z)
{
	uint8_t space = b->length - b->fill;
	if(space < LPP_ACCELEROMETER_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_ACCELEROMETER_CHANNEL;
	b->buffer[b->fill++] = LPP_ACCELEROMETER;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & x) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & x);
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & y) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & y);
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & z) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & z);

	return (true);
}

bool LPP_AddPressure(LPP_Buffer_t *b, uint16_t data)
{
	uint8_t space = b->length - b->fill;
	if(space < LPP_PRESSURE_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_PRESSURE_CHANNEL;
	b->buffer[b->fill++] = LPP_PRESSURE;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & data) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & data);

	return (true);
}
