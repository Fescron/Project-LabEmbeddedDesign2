/***************************************************************************//**
 * @file datatypes.h
 * @brief Definitions of the custom data-types used.
 * @version 2.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Moved `mcu_states` enum definition from `main.c` to this file and added `MeasurementData_t` struct data type.
 *   @li v1.1: Added another `MCU_State_t` option.
 *   @li v1.2: Added another `MCU_State_t` option.
 *   @li v1.3: Changed data types in `MeasurementData_t` struct.
 *   @li v2.0: Updated version number.
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
#ifndef _DATATYPES_H_
#define _DATATYPES_H_


/* Include necessary for this header file */
#include <stdint.h> /* (u)intXX_t */


/** Enum type for the state machine */
typedef enum mcu_states
{
	INIT,
	MEASURE,
	SEND,
	SEND_STORM,
	SLEEP,
	SLEEP_HALFTIME,
	WAKEUP
} MCU_State_t;


/** Struct type to store the gathered data */
typedef struct
{
	uint8_t index;
	int32_t voltage[6];
	int32_t intTemp[6];
	int32_t extTemp[6];
} MeasurementData_t;


#endif /* _DATATYPES_H_ */
