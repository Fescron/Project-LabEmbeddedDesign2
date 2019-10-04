/***************************************************************************//**
 * @file dbprint.h
 * @brief Homebrew println/printf replacement "DeBugPrint".
 * @version 6.2
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
#ifndef _DBPRINT_H_
#define _DBPRINT_H_


/* Includes necessary for this header file */
#include <stdint.h>   /* (u)intXX_t */
#include <stdbool.h>  /* "bool", "true", "false" */
#include "em_usart.h" /* Universal synchr./asynchr. receiver/transmitter (USART/UART) Peripheral API */


/** Public definition to configure the buffer size. */
#define DBPRINT_BUFFER_SIZE 80


/** Enum type for the color selection. */
typedef enum dbprint_colors
{
	RED,
	GREEN,
	BLUE,
	CYAN,
	MAGENTA,
	YELLOW,
	DEFAULT_COLOR
} dbprint_color_t;


/* Public prototypes */
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts);

void dbAlert (void);
void dbClear (void);

void dbprint (char *message);
void dbprintln (char *message);

void dbprintInt (int32_t value);
void dbprintlnInt (int32_t value);

void dbprintInt_hex (int32_t value);
void dbprintlnInt_hex (int32_t value);

void dbprint_color (char *message, dbprint_color_t color);
void dbprintln_color (char *message, dbprint_color_t color);

void dbinfo (char *message);
void dbwarn (char *message);
void dbcrit (char *message);

void dbinfoInt (char *message1, int32_t value, char *message2);
void dbwarnInt (char *message1, int32_t value, char *message2);
void dbcritInt (char *message1, int32_t value, char *message2);

void dbinfoInt_hex (char *message1, int32_t value, char *message2);
void dbwarnInt_hex (char *message1, int32_t value, char *message2);
void dbcritInt_hex (char *message1, int32_t value, char *message2);

char dbReadChar (void);
uint8_t dbReadInt (void);
void dbReadLine (char *buf);

bool dbGet_RXstatus (void);
// void dbSet_TXbuffer (char *message); // TODO: Needs fixing (but probably won't ever be used)
void dbGet_RXbuffer (char *buf);


#endif /* _DBPRINT_H_ */
