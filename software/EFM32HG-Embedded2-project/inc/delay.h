/***************************************************************************//**
 * @file delay.h
 * @brief Delay functionality.
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
#ifndef _DELAY_H_
#define _DELAY_H_


/* Includes necessary for this header file */
#include <stdint.h> /* (u)intXX_t */


/** Public definition to select which delay to use
 *    @li `1` - Use SysTick delays.
 *    @li `0` - Use EM2/3 RTC compare delays. */
#define SYSTICKDELAY 0


/** Public definition to select the use of the crystal or the oscillator
 *    @li `0` - Use the low-frequency crystal oscillator (LFXO), EM2 sleep is used.
 *    @li `1` - Use the ultra low-frequency RC oscillator (ULFRCO), EM3 sleep is used but delays are less precise timing-wise.
 *              ** EM3: All unwanted oscillators are disabled, they don't need to manually disabled before `EMU_EnterEM3`.**    */
#define ULFRCO 1


/* Public prototypes */
void delay (uint32_t msDelay);
void sleep (uint32_t sSleep);
bool RTC_checkWakeup (void);
void RTC_clearWakeup (void);
uint32_t RTC_getPassedSleeptime (void);


#endif /* _DELAY_H_ */
