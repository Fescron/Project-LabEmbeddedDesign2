/***************************************************************************//**
 * @file debug_dbprint.h
 * @brief Enable or disable printing to UART with dbprint.
 * @details
 *   **This header file should be called in every other file where there are UART
 *   dbprint debugging statements. Depending on the value of `DEBUG_DBPRINT`,
 *   UART statements are enabled or disabled.**
 * @version 6.0
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
 *   Some methods also use code obtained from examples from [Silicon Labs' GitHub](https://github.com/SiliconLabs/peripheral_examples).
 *   These sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DEBUG_DBPRINT_H_
#define _DEBUG_DBPRINT_H_


/** Public definition to enable/disable UART debugging
 *    @li `1` - Enable the UART debugging statements.
 *    @li `0` - Remove all UART debugging statements from the uploaded code. */
#define DEBUG_DBPRINT 1


#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
#include "dbprint.h"
#endif /* DEBUG_DBPRINT */


#endif /* _DEBUG_DBPRINT_H_ */
