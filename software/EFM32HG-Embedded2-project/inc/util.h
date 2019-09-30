/***************************************************************************//**
 * @file util.h
 * @brief Utility functionality.
 * @version 3.0
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
#ifndef _UTIL_H_
#define _UTIL_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/** Public definition to enable/disable the logic to send certain error call values (everything except 30 - 50) to the cloud using LoRaWAN.
 *    @li `0` - Keep the MCU in a `while(true)` loop if the `error` method is called while flashing the LED and displaying a UART message.
 *    @li `1` - Display a UART message when the `error` method is called but also forward the number to the cloud using LoRaWAN. Don't go in a `while(true)` loop.  */
#define ERROR_FORWARDING 1


/* Public prototypes */
void led (bool enabled);
void error (uint8_t number);


#endif /* _UTIL_H_ */
