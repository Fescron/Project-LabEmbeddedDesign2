/***************************************************************************//**
 * @file ADXL362.h
 * @brief All code for the ADXL362 accelerometer.
 * @version 3.1
 * @author Brecht Van Eeckhoudt
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
 *   Some methods use code obtained from examples from [Silicon Labs' GitHub](https://github.com/SiliconLabs/peripheral_examples).
 *   These sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _ADXL362_H_
#define _ADXL362_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/** Enum type for the measurement range */
typedef enum adxl_range
{
	ADXL_RANGE_2G, /* +- 2g (reset default) */
	ADXL_RANGE_4G, /* +- 4g */
	ADXL_RANGE_8G  /* +- 8g */
} ADXL_Range_t;

/** Enum type for the ODR */
typedef enum adxl_odr
{
	ADXL_ODR_12_5_HZ, /* 12.5 Hz */
	ADXL_ODR_25_HZ,   /* 25 Hz */
	ADXL_ODR_50_HZ,   /* 50 Hz */
	ADXL_ODR_100_HZ,  /* 100 Hz (reset default) */
	ADXL_ODR_200_HZ,  /* 200 Hz */
	ADXL_ODR_400_HZ   /* 400 Hz */
} ADXL_ODR_t;


/* Public prototypes */
void initADXL (void);

void ADXL_setTriggered (bool triggered);
bool ADXL_getTriggered (void);
void ADXL_ackInterrupt (void);

uint16_t ADXL_getCounter (void);
void ADXL_clearCounter (void);

void ADXL_enableSPI (bool enabled);
void ADXL_enableMeasure (bool enabled);

void ADXL_configRange (ADXL_Range_t givenRange);
void ADXL_configODR (ADXL_ODR_t givenODR);
void ADXL_configActivity (uint8_t gThreshold);

void ADXL_readValues (void);

void testADXL (void);


#endif /* _ADXL362_H_ */
