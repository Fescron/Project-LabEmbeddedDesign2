/***************************************************************************//**
 * @file lpp.h
 * @brief Basic Low Power Payload (LPP) functionality.
 * @version 1.1
 * @author
 *   Geoffrey Ottoy@n
 *   Modified by Brecht Van Eeckhoudt
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
 *         File: lpp.h
 *      Created: 2018-03-23
 *       Author: Geoffrey Ottoy
 *
 *  Description: Header file for lpp.c
 */


#ifndef _LPP_H_
#define _LPP_H_

#include <em_device.h>
#include <stdbool.h>

typedef struct lpp_buffer{
	uint8_t * buffer;
	uint8_t fill;
	uint8_t length;
} LPP_Buffer_t;

bool LPP_InitBuffer(LPP_Buffer_t * b, uint8_t size);
void LPP_ClearBuffer(LPP_Buffer_t * b);

bool LPP_AddVBAT (LPP_Buffer_t *b, int16_t vbat);
bool LPP_AddIntTemp (LPP_Buffer_t *b, int16_t intTemp);
bool LPP_AddExtTemp (LPP_Buffer_t *b, int16_t extTemp);
bool LPP_AddStormDetected (LPP_Buffer_t *b, uint8_t stormDetected);
bool LPP_AddCableBroken (LPP_Buffer_t *b, uint8_t cableValue);
bool LPP_AddStatus (LPP_Buffer_t *b, uint8_t status);

bool LPP_AddDigital(LPP_Buffer_t *b, uint8_t data);
bool LPP_AddAnalog(LPP_Buffer_t *b, int16_t data);
bool LPP_AddTemperature(LPP_Buffer_t *b, int16_t data);
bool LPP_AddHumidity(LPP_Buffer_t *b, uint8_t data);
bool LPP_AddAccelerometer(LPP_Buffer_t *b, int16_t x, int16_t y, int16_t z);
bool LPP_AddPressure(LPP_Buffer_t *b, uint16_t data);

#endif /* _LPP_H_ */
