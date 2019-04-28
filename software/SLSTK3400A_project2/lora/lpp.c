/***************************************************************************//**
 * @file lpp.c
 * @brief Basic Low Power Payload (LPP) functionality.
 * @version 1.1
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

#include <stdlib.h>
#include <stdbool.h>

#include "lpp.h"

// lpp types
#define LPP_DIGITAL_INPUT			0x00
#define LPP_ANALOG_INPUT			0x02
#define LPP_TEMPERATURE				0x67
#define LPP_HUMIDITY				0x68
#define LPP_ACCELEROMETER			0x71
#define LPP_PRESSURE				0x73

// lpp data sizes
#define LPP_DIGITAL_INPUT_SIZE		0x03
#define LPP_ANALOG_INPUT_SIZE		0x04
#define LPP_TEMPERATURE_SIZE		0x04
#define LPP_HUMIDITY_SIZE			0x03
#define LPP_ACCELEROMETER_SIZE		0x08
#define LPP_PRESSURE_SIZE			0x04

// lpp channel ids
#define LPP_DIGITAL_INPUT_CHANNEL	0x01
#define LPP_ANALOG_INPUT_CHANNEL	0x02
#define LPP_TEMPERATURE_CHANNEL		0x03
#define LPP_HUMIDITY_CHANNEL		0x04
#define LPP_ACCELEROMETER_CHANNEL	0x05
#define LPP_PRESSURE_CHANNEL		0x06

/* Custom channel ID's */
#define LPP_VBAT_CHANNEL            0x10
#define LPP_TEMPERATURE_CHANNEL_INT 0x11
#define LPP_TEMPERATURE_CHANNEL_EXT 0x12
#define LPP_STORM_CHANNEL           0x13
#define LPP_CABLE_BROKEN_CHANNEL    0x14
#define LPP_STATUS_CHANNEL          0x15


bool LPP_InitBuffer(LPP_Buffer_t *b, uint8_t size){
	LPP_ClearBuffer(b);

	b->buffer = (uint8_t *) malloc(sizeof(uint8_t) * size);

	if(b->buffer != NULL){
		b->fill = 0;
		b->length = size;
		return (true);
	}

	return (false);
}

void LPP_ClearBuffer(LPP_Buffer_t *b){
	if(b->buffer != NULL){
		free(b->buffer);
	}
}

/**************************************************************************//**
 * @brief
 *   Add a battery voltage measurement to the LPP packet, disguised as an
 *   *Analog Input* packet (2 bytes). The channel is defined by `LPP_VBAT_CHANNEL`
 *   and is `0x10`.
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
bool LPP_AddVBAT (LPP_Buffer_t *b, int16_t vbat)
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
bool LPP_AddIntTemp (LPP_Buffer_t *b, int16_t intTemp)
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
bool LPP_AddExtTemp (LPP_Buffer_t *b, int16_t extTemp)
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
	uint8_t space = b->length - b->fill;
	if (space < LPP_DIGITAL_INPUT_SIZE) return (false);

	b->buffer[b->fill++] = LPP_STATUS_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = status;

	return (true);
}

bool LPP_AddDigital(LPP_Buffer_t *b, uint8_t data){
	uint8_t space = b->length - b->fill;
	if(space < LPP_DIGITAL_INPUT_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_DIGITAL_INPUT_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = data;

	return (true);
}

bool LPP_AddAnalog(LPP_Buffer_t *b, int16_t data){
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

bool LPP_AddTemperature(LPP_Buffer_t *b, int16_t data){
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

bool LPP_AddHumidity(LPP_Buffer_t *b, uint8_t data){
	uint8_t space = b->length - b->fill;
	if(space < LPP_HUMIDITY_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = LPP_HUMIDITY_CHANNEL;
	b->buffer[b->fill++] = LPP_HUMIDITY;
	b->buffer[b->fill++] = data;

	return (true);
}

bool LPP_AddAccelerometer(LPP_Buffer_t *b, int16_t x, int16_t y, int16_t z){
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

bool LPP_AddPressure(LPP_Buffer_t *b, uint16_t data){
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
