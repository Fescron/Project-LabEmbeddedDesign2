/***************************************************************************//**
 * @file dbprint.c
 * @brief Homebrew println/printf replacement "DeBugPrint".
 * @details Originally designed for use on the Silicion Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48).
 * @version 6.2
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   Please check https://github.com/Fescron/dbprint to find the latest version of dbprint!
 *
 *   @li v1.0: "define" used to jump between VCOM or other mode, itoa (<stdlib.h>) used aswell as stdio.h
 *   @li v1.1: Separated printInt method in a separate function for printing "int32_t" and "uint32_t" values.
 *   @li v1.2: Added more options to the initialize method (location selection & boolean if VCOM is used).
 *   @li v2.0: Restructured files to be used in other projects, added a lot more documentation, added "dbAlert" and "dbClear" methods.
 *   @li v2.1: Added interrupt functionality.
 *   @li v2.2: Added parse functions, separated method for printing uint values in a separate one for DEC and HEX notation.
 *   @li v2.3: Updated documentation.
 *   @li v2.4: Fixed notes.
 *   @li v2.5: Separated method for printing int values in a separate one for DEC and HEX notation.
 *   @li v2.6: Stopped using itoa (<stdlib.h>) in all methods.
 *   @li v3.0: Simplified number printing, stopped using separate methods for uint and int values.
 *   @li v3.1: Removed useless if... check.
 *   @li v3.2: Added the ability to print text in a color.
 *   @li v3.3: Added info, warning and critical error printing methods.
 *   @li v3.4: Added printInt(_hex) methods that directly go to a new line.
 *   @li v3.5: Added USART0 IRQ handlers.
 *   @li v3.6: Added the ability to print (u)int values as INFO, WARN or CRIT lines.
 *   @li v3.7: Added separate "_hex" methods for dbinfo/warn/critInt instead of a boolean to select (hexa)decimal notation.
 *   @li v3.8: Added ReadChar-Int-Line methods.
 *   @li v3.9: Added "void" between argument brackets were before nothing was.
 *   @li v4.0: Added more documentation.
 *   @li v4.1: Added color reset before welcome message.
 *   @li v5.0: Made uint-char conversion methods static, moved color functionality to enum,
 *             started moving interrupt functionality to use getters and setters.
 *   @li v5.1: Fixed interrupt functionality with getters and setters.
 *   @li v6.0: Cleaned up interrupt functionality and added some documentation.
 *             (put some code in comments to maybe fix later if ever necessary)
 *   @li v6.1: Made `dbpointer` a local variable (removed `extern`).
 *             Removed `extern` from the documentation.
 *   @li v6.2: Removed `static` before the local variables (not necessary).
 *
 * ******************************************************************************
 *
 * @todo
 *   **Future improvements:**@n
 *     - Fix `dbSet_TXbuffer` and also add more functionality to print numbers, ...
 *     - Separate back-end <-> MCU specific code
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
 * ******************************************************************************
 *
 * @attention
 *   See the file `dbprint_documentation.h` for a lot of useful documentation!
 *
 ******************************************************************************/


#include "debug_dbprint.h" /* Enable or disable printing to UART for debugging */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */


#include <stdint.h>        /* (u)intXX_t */
#include <stdbool.h>       /* "bool", "true", "false" */
#include "em_cmu.h"        /* Clock Management Unit */
#include "em_gpio.h"       /* General Purpose IO (GPIO) peripheral API */
#include "em_usart.h"      /* Universal synchr./asynchr. receiver/transmitter (USART/UART) Peripheral API */


/* Local definitions */
/* Macro definitions that return a character when given a value */
#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i) /* "?:" = ternary operator (return ['0' + i] if [i <= 9] = true, ['A' - 10 + i] if false) */
#define TO_DEC(i) (i <= 9 ? '0' + i : '?') /* return "?" if out of range */

/* ANSI colors */
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"


/** Local variable to store the settings (pointer). */
USART_TypeDef* dbpointer;

/* Local variables to store data */
/*   -> Volatile because it's modified by an interrupt service routine (@RAM) */
volatile bool dataReceived = false; /* true if there is a line of data received */
volatile char rx_buffer[DBPRINT_BUFFER_SIZE];
volatile char tx_buffer[DBPRINT_BUFFER_SIZE];


/* Local prototypes */
static void uint32_to_charHex (char *buf, uint32_t value, bool spacing);
static void uint32_to_charDec (char *buf, uint32_t value);
static uint32_t charDec_to_uint32 (char *buf);
//static uint32_t charHex_to_uint32 (char *buf); // Unused but kept here just in case


/**************************************************************************//**
 * @brief
 *   Initialize USARTx.
 *
 * @param[in] pointer
 *   Pointer to USARTx.
 *
 * @param[in] location
 *   Location for pin routing.
 *
 * @param[in] vcom
 *   @li `true` - Isolation switch enabled by setting `PA9` high so the **Virtual COM port (CDC)** can be used.
 *   @li `false` - Isolation switch disabled on the Happy Gecko board.
 *
 * @param[in] interrupts
 *   @li `true` - Enable interrupt functionality.
 *   @li `false` - No interrupt functionality is initialized.
 *****************************************************************************/
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts)
{
	/* Store the pointer in the global variable */
	dbpointer = pointer;

	/*
	 * USART_INITASYNC_DEFAULT:
	 *   config.enable = usartEnable       // Specifies whether TX and/or RX is enabled when initialization is completed
	 *                                     // (Enable RX/TX when initialization is complete).
	 *   config.refFreq = 0                // USART/UART reference clock assumed when configuring baud rate setup
	 *                                     // (0 = Use current configured reference clock for configuring baud rate).
	 *   config.baudrate = 115200          // Desired baudrate (115200 bits/s).
	 *   config.oversampling = usartOVS16  // Oversampling used (16x oversampling).
	 *   config.databits = usartDatabits8  // Number of data bits in frame (8 data bits).
	 *   config.parity = usartNoParity     // Parity mode to use (no parity).
	 *   config.stopbits = usartStopbits1  // Number of stop bits to use (1 stop bit).
	 *   config.mvdis = false              // Majority Vote Disable for 16x, 8x and 6x oversampling modes (Do not disable majority vote).
	 *   config.prsRxEnable = false        // Enable USART Rx via PRS (Not USART PRS input mode).
	 *   config.prsRxCh = 0                // Select PRS channel for USART Rx. (Only valid if prsRxEnable is true - PRS channel 0).
	 *   config.autoCsEnable = false       // Auto CS enabling (Auto CS functionality enable/disable switch - disabled).
	 */

	USART_InitAsync_TypeDef config = USART_INITASYNC_DEFAULT;

	/* Enable oscillator to GPIO*/
	CMU_ClockEnable(cmuClock_GPIO, true);


	/* Enable oscillator to USARTx modules */
	if (dbpointer == USART0)
	{
		CMU_ClockEnable(cmuClock_USART0, true);
	}
	else if (dbpointer == USART1)
	{
		CMU_ClockEnable(cmuClock_USART1, true);
	}


	/* Set PA9 (EFM_BC_EN) high if necessary to enable the isolation switch */
	if (vcom)
	{
		GPIO_PinModeSet(gpioPortA, 9, gpioModePushPull, 1);
		GPIO_PinOutSet(gpioPortA, 9);
	}


	/* Set pin modes for UART TX and RX pins */
	if (dbpointer == USART0)
	{
		switch (location)
		{
			case 0:
				GPIO_PinModeSet(gpioPortE, 11, gpioModeInput, 0);    /* RX */
				GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 1); /* TX */
				break;
			case 2:
				GPIO_PinModeSet(gpioPortC, 10, gpioModeInput, 0);    /* RX */
				/* No TX pin in this mode */
				break;
			case 3:
				GPIO_PinModeSet(gpioPortE, 12, gpioModeInput, 0);    /* RX */
				GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1); /* TX */
				break;
			case 4:
				GPIO_PinModeSet(gpioPortB, 8, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortB, 7, gpioModePushPull, 1);  /* TX */
				break;
			case 5:
			case 6:
				GPIO_PinModeSet(gpioPortC, 1, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1);  /* TX */
				break;
			/* default: */
				/* No default */
		}
	}
	else if (dbpointer == USART1)
	{
		switch (location)
		{
			case 0:
				GPIO_PinModeSet(gpioPortC, 1, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1);  /* TX */
				break;
			case 2:
			case 3:
				GPIO_PinModeSet(gpioPortD, 6, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 1);  /* TX */
				break;
			case 4:
				GPIO_PinModeSet(gpioPortA, 0, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortF, 2, gpioModePushPull, 1);  /* TX */
				break;
			case 5:
				GPIO_PinModeSet(gpioPortC, 2, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 1);  /* TX */
				break;
			/* default: */
				/* No default */
		}
	}


	/* Initialize USART asynchronous mode */
	USART_InitAsync(dbpointer, &config);

	/* Route pins */
	switch (location)
	{
		case 0:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC0;
			break;
		case 1:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC1;
			break;
		case 2:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC2;
			break;
		case 3:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC3;
			break;
		case 4:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC4;
			break;
		case 5:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC5;
			break;
		case 6:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC6;
			break;
		default:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_DEFAULT;
	}

	/* Enable interrupts if necessary and print welcome string (and make an alert sound in the console) */
	if (interrupts)
	{
		/* Initialize USART interrupts */

		/* RX Data Valid Interrupt Enable
		 *   Set when data is available in the receive buffer. Cleared when the receive buffer is empty. */
		USART_IntEnable(dbpointer, USART_IEN_RXDATAV);

		/* TX Complete Interrupt Enable
		 *   Set when a transmission has completed and no more data is available in the transmit buffer.
		 *   Cleared when a new transmission starts. */
		USART_IntEnable(dbpointer, USART_IEN_TXC);

		if (dbpointer == USART0)
		{
			/* Enable USART interrupts */
			NVIC_EnableIRQ(USART0_RX_IRQn);
			NVIC_EnableIRQ(USART0_TX_IRQn);
		}
		else if (dbpointer == USART1)
		{
			/* Enable USART interrupts */
			NVIC_EnableIRQ(USART1_RX_IRQn);
			NVIC_EnableIRQ(USART1_TX_IRQn);
		}

		/* Print welcome string */
		dbprint(COLOR_RESET);
		dbprintln("\a\r\f### UART initialized (interrupt mode) ###");
		dbinfo("This is an info message.");
		dbwarn("This is a warning message.");
		dbcrit("This is a critical error message.");
		dbprintln("###  Start executing programmed code  ###\n");

		/* Set TX Complete Interrupt Flag (transmission has completed and no more data
		* is available in the transmit buffer) */
		USART_IntSet(dbpointer, USART_IFS_TXC);
	}
	/* Print welcome string (and make an alert sound in the console) if not in interrupt mode */
	else
	{
		dbprint(COLOR_RESET);
		dbprintln("\a\r\f### UART initialized (no interrupts) ###");
		dbinfo("This is an info message.");
		dbwarn("This is a warning message.");
		dbcrit("This is a critical error message.");
		dbprintln("### Start executing programmed code  ###\n");
	}
}


/**************************************************************************//**
 * @brief
 *   Sound an alert in the terminal.
 *
 * @details
 *   Print the *bell* (alert) character to USARTx.
 *****************************************************************************/
void dbAlert (void)
{
	USART_Tx(dbpointer, '\a');
}


/**************************************************************************//**
 * @brief
 *   Clear the terminal.
 *
 * @details
 *   Print the *form feed* character to USARTx. Accessing old data is still
 *   possible by scrolling up in the serial port program.
 *****************************************************************************/
void dbClear (void)
{
	USART_Tx(dbpointer, '\f');
}


/**************************************************************************//**
 * @brief
 *   Print a string (char array) to USARTx.
 * 
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbprint (char *message)
{
	/* "message[i] != 0" makes "uint32_t length = strlen(message)"
	 * not necessary (given string MUST be terminated by NULL for this to work) */
	for (uint32_t i = 0; message[i] != 0; i++)
	{
		USART_Tx(dbpointer, message[i]);
	}
}


/**************************************************************************//**
 * @brief
 *   Print a string (char array) to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbprintln (char *message)
{
	dbprint(message);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief
 *   Print a string (char array) to USARTx in a given color.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *
 * @param[in] color
 *   The color to print the text in.
 *****************************************************************************/
void dbprint_color (char *message, dbprint_color_t color)
{
	switch (color)
	{
		case DEFAULT_COLOR:
			dbprint(COLOR_RESET);
			dbprint(message);
			break;
		case RED:
			dbprint(COLOR_RED);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case GREEN:
			dbprint(COLOR_GREEN);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case BLUE:
			dbprint(COLOR_BLUE);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case CYAN:
			dbprint(COLOR_CYAN);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case MAGENTA:
			dbprint(COLOR_MAGENTA);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case YELLOW:
			dbprint(COLOR_YELLOW);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		default:
			dbprint(COLOR_RESET);
			dbprint(message);
	}
}


/**************************************************************************//**
 * @brief
 *   Print a string (char array) to USARTx in a given color and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *
 * @param[in] color
 *   The color to print the text in.
 *****************************************************************************/
void dbprintln_color (char *message, dbprint_color_t color)
{
	dbprint_color(message, color);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief
 *   Print an *info* string (char array) to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbinfo (char *message)
{
	dbprint("INFO: ");
	dbprintln(message);
}


/**************************************************************************//**
 * @brief
 *   Print a *warning* string (char array) in yellow to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbwarn (char *message)
{
	dbprint_color("WARN: ", YELLOW);
	dbprintln_color(message, YELLOW);
}


/**************************************************************************//**
 * @brief
 *   Print a *critical error* string (char array) in red to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbcrit (char *message)
{
	dbprint_color("CRIT: ", RED);
	dbprintln_color(message, RED);
}


/**************************************************************************//**
 * @brief
 *   Print an *info* value surrounded by two strings (char array) to USARTx.
 *
 * @details
 *   "INFO: " gets added in front, the decimal notation is used and
 *   the function advances to the next line.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbinfoInt (char *message1, int32_t value, char *message2)
{
	dbprint("INFO: ");
	dbprint(message1);
	dbprintInt(value);
	dbprintln(message2);
}


/**************************************************************************//**
 * @brief
 *   Print a *warning* value surrounded by two strings (char array) to USARTx.
 *
 * @details
 *   "WARN: " gets added in front, the decimal notation is used and
 *   the function advances to the next line. The value is in the color white,
 *   the rest is yellow.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbwarnInt (char *message1, int32_t value, char *message2)
{
	dbprint_color("WARN: ", YELLOW);
	dbprint_color(message1, YELLOW);
	dbprintInt(value);
	dbprintln_color(message2, YELLOW);
}


/**************************************************************************//**
 * @brief
 *   Print a *critical* value surrounded by two strings (char array) to USARTx.
 *
 * @details
 *   "CRIT: " gets added in front, the decimal notation is used and
 *   the function advances to the next line. The value is in the color white,
 *   the rest is red.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbcritInt (char *message1, int32_t value, char *message2)
{
	dbprint_color("CRIT: ", RED);
	dbprint_color(message1, RED);
	dbprintInt(value);
	dbprintln_color(message2, RED);
}


/**************************************************************************//**
 * @brief
 *   Print an *info* value surrounded by two strings (char array) to USARTx.
 *
 * @details
 *   "INFO: " gets added in front, the hexadecimal notation is used and
 *   the function advances to the next line.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbinfoInt_hex (char *message1, int32_t value, char *message2)
{
	dbprint("INFO: ");
	dbprint(message1);
	dbprintInt_hex(value);
	dbprintln(message2);
}


/**************************************************************************//**
 * @brief
 *   Print a *warning* value surrounded by two strings (char array) to USARTx.
 *
 * @details
 *   "WARN: " gets added in front, the hexadecimal notation is used and
 *   the function advances to the next line. The value is in the color white,
 *   the rest is yellow.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbwarnInt_hex (char *message1, int32_t value, char *message2)
{
	dbprint_color("WARN: ", YELLOW);
	dbprint_color(message1, YELLOW);
	dbprintInt_hex(value);
	dbprintln_color(message2, YELLOW);
}


/**************************************************************************//**
 * @brief
 *   Print a *critical* value surrounded by two strings (char array) to USARTx.
 *
 * @details
 *   "CRIT: " gets added in front, the hexadecimal notation is used and
 *   the function advances to the next line. The value is in the color white,
 *   the rest is red.
 *
 * @note
 *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
 *   the input message (array) needs to end with NULL (`"\0"`)!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbcritInt_hex (char *message1, int32_t value, char *message2)
{
	dbprint_color("CRIT: ", RED);
	dbprint_color(message1, RED);
	dbprintInt_hex(value);
	dbprintln_color(message2, RED);
}


/**************************************************************************//**
 * @brief
 *   Print a number in decimal notation to USARTx.
 *
 * @param[in] value
 *   The number to print to USARTx.@n
 *   This can be of type `uint32_t` or `int32_t`.
 *****************************************************************************/
void dbprintInt (int32_t value)
{
	/* Buffer to put the char array in (Needs to be 10) */
	char decchar[10];

	/* Convert a negative number to a positive one and print the "-" */
	if (value < 0)
	{
		/* Negative of value = flip all bits, +1
		 *   bitwise logic: "~" = "NOT" */
		uint32_t negativeValue = (~value) + 1;

		dbprint("-");

		/* Convert the value */
		uint32_to_charDec(decchar, negativeValue);
	}
	else
	{
		/* Convert the value */
		uint32_to_charDec(decchar, value);
	}

	/* Print the buffer */
	dbprint(decchar);
}


/**************************************************************************//**
 * @brief
 *   Print a number in decimal notation to USARTx and go to the next line.
 *
 * @param[in] value
 *   The number to print to USARTx.@n
 *   This can be of type `uint32_t` or `int32_t`.
 *****************************************************************************/
void dbprintlnInt (int32_t value)
{
	dbprintInt(value);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief
 *   Print a number in hexadecimal notation to USARTx.
 *
 * @param[in] value
 *   The number to print to USARTx.@n
 *   This can be of type `uint32_t` or `int32_t`.
 *****************************************************************************/
void dbprintInt_hex (int32_t value)
{
	char hexchar[9]; /* Needs to be 9 */
	uint32_to_charHex(hexchar, value, true); /* true: add spacing between eight HEX chars */
	dbprint("0x");
	dbprint(hexchar);
}


/**************************************************************************//**
 * @brief
 *   Print a number in hexadecimal notation to USARTx and go to the next line.
 *
 * @param[in] value
 *   The number to print to USARTx.@n
 *   This can be of type `uint32_t` or `int32_t`.
 *****************************************************************************/
void dbprintlnInt_hex (int32_t value)
{
	dbprintInt_hex(value);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief
 *   Read a character from USARTx.
 *
 * @return
 *   The character read from USARTx.
 *****************************************************************************/
char dbReadChar (void)
{
	return (USART_Rx(dbpointer));
}


/**************************************************************************//**
 * @brief
 *   Read a decimal character from USARTx and convert it to a `uint8_t` value.
 *
 * @note
 *   Specific methods exist to read `uint16_t` and `uint32_t` values:
 *     - `uint16_t USART_RxDouble(USART_TypeDef *usart);`
 *     - `uint32_t USART_RxDoubleExt(USART_TypeDef *usart);`
 *
 * @return
 *   The converted `uint8_t` value.
 *****************************************************************************/
uint8_t dbReadInt (void)
{
	/* Method expects a char array ending with a null termination character */
	char value[2];
	value[0]= dbReadChar();
	value[1] = '\0';

	return (charDec_to_uint32(value));
}


/**************************************************************************//**
 * @brief
 *   Read a string (char array) from USARTx.
 *
 * @note
 *   The reading stops when a `"CR"` (Carriage Return, ENTER) character
 *   is received or the maximum length (`DBPRINT_BUFFER_SIZE`) is reached.
 *
 * @param[in] buf
 *   The buffer to put the resulting string in.@n
 *   **This needs to have a length of `DBPRINT_BUFFER_SIZE` for the function
 *   to work properly: `char buf[DBPRINT_BUFFER_SIZE];`!**
 *****************************************************************************/
void dbReadLine (char *buf)
{
	for (uint32_t i = 0; i < DBPRINT_BUFFER_SIZE - 1 ; i++ )
	{
		char localBuffer = USART_Rx(dbpointer);

		/* Check if a CR character is received */
		if (localBuffer == '\r')
		{
			/* End with a null termination character, expected by the dbprintln method */
			buf[i] = '\0';
			break;
		}
		else
		{
			buf[i] = localBuffer;
		}
	}
}


/**************************************************************************//**
 * @brief
 *   Check if data was received using interrupts in the RX buffer.
 *
 * @note
 *   The index of the RX buffer gets reset in the RX handler when a CR character
 *   is received or the buffer is filled.
 *
 * @attention
 *   Interrupt functionality has to be enabled on initialization for this
 *   function to work correctly.
 *
 * @return
 *   @li `true` - Data is received in the RX buffer.
 *   @li `false` - No data is received.
 *****************************************************************************/
bool dbGet_RXstatus (void)
{
	return (dataReceived);
}


// TODO: Needs fixing (but probably won't ever be used):
//**************************************************************************//**
// * @brief
// *   Set the value of the TX buffer and start transmitting it using interrupts.
// *
// * @details
// *   The `"TX Complete Interrupt Flag"` also gets set at the end of the
// *   function (transmission has completed and no more data is available
// *   in the transmit buffer).
// *
// * @note
// *   If the input is not a string (ex.: `"Hello world!"`) but a char array,
// *   the input message (array) needs to end with NULL (`"\0"`)!
// *
// * @note
// *   The index of the RX buffer gets reset in the TX handler when all the
// *   characters in the buffer are send.
// *
// * @attention
// *   Interrupt functionality has to be enabled on initialization for this
// *   function to work correctly.
// *
// * @param[in] message
// *   The string to put in the TX buffer.
// *****************************************************************************/
//void dbSet_TXbuffer (char *message)
//{
//	uint32_t i;
//
//	/* Copy data to the TX buffer */
//	for (i = 0; message[i] != 0 && i < DBPRINT_BUFFER_SIZE-1; i++)
//	{
//		tx_buffer[i] = message[i];
//	}
//
//	/* Set TX Complete Interrupt Flag (transmission has completed and no more data
//	* is available in the transmit buffer) */
//	USART_IntSet(dbpointer, USART_IFS_TXC);
//
// TODO: Perhaps some functionality needs to be added to send numbers and
//       that might need functionality to disable interrupts. For reference
//       some old code that could be put in a main.c file is put below.
//	/* Data is ready to retransmit (notified by the RX handler) */
//	if (dbprint_rxdata)
//	{
//		uint32_t i;
//
//		/* RX Data Valid Interrupt Enable
//		*   Set when data is available in the receive buffer. Cleared when the receive buffer is empty.
//		*
//		* TX Complete Interrupt Enable
//		*   Set when a transmission has completed and no more data is available in the transmit buffer.
//		*   Cleared when a new transmission starts.
//		*/
//
//		/* Disable "RX Data Valid Interrupt Enable" and "TX Complete Interrupt Enable" interrupts */
//		USART_IntDisable(dbpointer, USART_IEN_RXDATAV);
//		USART_IntDisable(dbpointer, USART_IEN_TXC);
//
//		/* Copy data from the RX buffer to the TX buffer */
//		for (i = 0; dbprint_rx_buffer[i] != 0 && i < DBPRINT_BUFFER_SIZE-3; i++)
//		{
//			dbprint_tx_buffer[i] = dbprint_rx_buffer[i];
//		}
//
//		/* Add "new line" characters */
//		dbprint_tx_buffer[i++] = '\r';
//		dbprint_tx_buffer[i++] = '\n';
//		dbprint_tx_buffer[i] = '\0';
//
//		/* Reset "notification" variable */
//		dbprint_rxdata = false;
//
//		/* Enable "RX Data Valid Interrupt" and "TX Complete Interrupt" interrupts */
//		USART_IntEnable(dbpointer, USART_IEN_RXDATAV);
//		USART_IntEnable(dbpointer, USART_IEN_TXC);
//
//		/* Set TX Complete Interrupt Flag (transmission has completed and no more data
//		* is available in the transmit buffer) */
//		USART_IntSet(dbpointer, USART_IFS_TXC);
//	}
//
//}


/**************************************************************************//**
 * @brief
 *   Get the value of the RX buffer and clear the `dataReceived` flag.
 *
 * @note
 *   The index of the RX buffer gets reset in the RX handler when a CR character
 *   is received or the buffer is filled.
 *
 * @attention
 *   Interrupt functionality has to be enabled on initialization for this
 *   function to work correctly.
 *
 * @param[in] buf
 *   The buffer to put the resulting string in.@n
 *   **This needs to have a length of `DBPRINT_BUFFER_SIZE` for the function
 *   to work properly: `char buf[DBPRINT_BUFFER_SIZE];`!**
 *****************************************************************************/
void dbGet_RXbuffer (char *buf)
{
	if (dataReceived)
	{
		uint32_t i;

		/* Copy data from the RX buffer to the given buffer */
		for (i = 0; rx_buffer[i] != 0 && i < DBPRINT_BUFFER_SIZE-1; i++)
		{
			buf[i] = rx_buffer[i];
		}

		/* Add NULL termination character */
		buf[i++] = '\0';

		/* Reset "notification" variable */
		dataReceived = false;
	}
	else
	{
		dbcrit("No received data available!");
	}
}


/**************************************************************************//**
 * @brief
 *   Convert a `uint32_t` value to a hexadecimal char array (string).
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[out] buf
 *   The buffer to put the resulting string in.@n
 *   **This needs to have a length of 9: `char buf[9];`!**
 *
 * @param[in] value
 *   The `uint32_t` value to convert to a string.
 *
 * @param[in] spacing
 *   @li `true` - Add spacing between the eight HEX chars to make two groups of four.
 *   @li `false` - Don't add spacing between the eight HEX chars.
 *****************************************************************************/
static void uint32_to_charHex (char *buf, uint32_t value, bool spacing)
{
	/* 4 nibble HEX representation */
	if (value <= 0xFFFF)
	{
		/* Only get necessary nibble by ANDing with a mask and
		 * shifting one nibble (4 bits) per position */
		buf[0] = TO_HEX(((value & 0xF000) >> 12));
		buf[1] = TO_HEX(((value & 0x0F00) >> 8 ));
		buf[2] = TO_HEX(((value & 0x00F0) >> 4 ));
		buf[3] = TO_HEX( (value & 0x000F)       );
		buf[4] = '\0'; /* NULL termination character */
	}

	/* 8 nibble HEX representation */
	else
	{
		/* Only get necessary nibble by ANDing with a mask and
		 * shifting one nibble (4 bits) per position */
		buf[0] = TO_HEX(((value & 0xF0000000) >> 28));
		buf[1] = TO_HEX(((value & 0x0F000000) >> 24));
		buf[2] = TO_HEX(((value & 0x00F00000) >> 20));
		buf[3] = TO_HEX(((value & 0x000F0000) >> 16));

		/* Add spacing if necessary */
		if (spacing)
		{
			buf[4] = ' '; /* Spacing */
			buf[5] = TO_HEX(((value & 0x0000F000) >> 12));
			buf[6] = TO_HEX(((value & 0x00000F00) >> 8 ));
			buf[7] = TO_HEX(((value & 0x000000F0) >> 4 ));
			buf[8] = TO_HEX( (value & 0x0000000F)       );
			buf[9] = '\0'; /* NULL termination character */
		}
		else
		{
			buf[4] = TO_HEX(((value & 0x0000F000) >> 12));
			buf[5] = TO_HEX(((value & 0x00000F00) >> 8 ));
			buf[6] = TO_HEX(((value & 0x000000F0) >> 4 ));
			buf[7] = TO_HEX( (value & 0x0000000F)       );
			buf[8] = '\0'; /* NULL termination character */
		}
	}
}


/**************************************************************************//**
 * @brief
 *   Convert a `uint32_t` value to a decimal char array (string).
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[out] buf
 *   The buffer to put the resulting string in.@n
 *   **This needs to have a length of 10: `char buf[10];`!**
 *
 * @param[in] value
 *   The `uint32_t` value to convert to a string.
 *****************************************************************************/
static void uint32_to_charDec (char *buf, uint32_t value)
{
	if (value == 0)
	{
		buf[0] = '0';
		buf[1] = '\0'; /* NULL termination character */
	}
	else
	{
		/* MAX uint32_t value = FFFFFFFFh = 4294967295d (10 decimal chars) */
		char backwardsBuf[10];

		uint32_t calcval = value;
		uint8_t length = 0;
		uint8_t lengthCounter = 0;


		/* Loop until the value is zero (separate characters 0-9) and calculate length */
		while (calcval)
		{
			uint32_t rem = calcval % 10;
			backwardsBuf[length] = TO_DEC(rem); /* Convert to ASCII character */
			length++;

			calcval = calcval - rem;
			calcval = calcval / 10;
		}

		/* Backwards counter */
		lengthCounter = length;

		/* Reverse the characters in the buffer for the final string */
		for (uint8_t i = 0; i < length; i++)
		{
			buf[i] = backwardsBuf[lengthCounter-1];
			lengthCounter--;
		}

		/* Add NULL termination character */
		buf[length] = '\0';
	}
}


/**************************************************************************//**
 * @brief
 *   Convert a string (char array) in decimal notation to a `uint32_t` value.
 *
 * @note
 *   If the input is not a string (ex.: `"00BA0FA1"`) but a char array,
 *   the input buffer (array) needs to end with NULL (`"\0"`)!
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] buf
 *   The decimal string to convert to a `uint32_t` value.
 *
 * @return
 *   The resulting `uint32_t` value.
 *****************************************************************************/
static uint32_t charDec_to_uint32 (char *buf)
{
	/* Value to eventually return */
	uint32_t value = 0;

	/* Loop until buffer is empty */
	while (*buf)
	{
		/* Get current character, increment afterwards */
		uint8_t byte = *buf++;

		/* Check if the next value we need to add can fit in a uint32_t */
		if ( (value <= 0xFFFFFFF) && ((byte - '0') <= 0xF) )
		{
			/* Convert the ASCII (decimal) character to the representing decimal value
			 * and add it to the value (which is multiplied by 10 for each position) */
			value = (value * 10) + (byte - '0');
		}
		else
		{
			/* Given buffer can't fit in uint32_t */
			return (0);
		}
	}

	return (value);
}


// Unused but kept here just in case:
//**************************************************************************//**
// * @brief
// *   Convert a string (char array) in hexadecimal notation to a `uint32_t` value.
// *
// * @note
// *   If the input is not a string (ex.: `"00120561"`) but a char array,
// *   the input buffer (array) needs to end with NULL (`"\0"`)!@n
// *   The input string can't have the prefix `0x`.
// *
// * @note
// *   This is a static method because it's only internally used in this file
// *   and called by other methods if necessary.
// *
// * @param[in] buf
// *   The hexadecimal string to convert to a `uint32_t` value.
// *
// * @return
// *   The resulting `uint32_t` value.
// *****************************************************************************/
//static uint32_t charHex_to_uint32 (char *buf)
//{
//	/* Value to eventually return */
//	uint32_t value = 0;
//
//	/* Loop until buffer is empty */
//	while (*buf)
//	{
//		/* Get current character, increment afterwards */
//		uint8_t byte = *buf++;
//
//		/* Convert the hex character to the 4-bit equivalent
//		 * number using the ASCII table indexes */
//		if (byte >= '0' && byte <= '9') byte = byte - '0';
//		else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
//		else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
//
//		/* Check if the next byte we need to add can fit in a uint32_t */
//		if ( (value <= 0xFFFFFFF) && (byte <= 0xF) )
//		{
//			/* Shift one nibble (4 bits) to make space for a new digit
//			 * and add the 4 bits (ANDing with a mask, 0xF = 0b1111) */
//			value = (value << 4) | (byte & 0xF);
//		}
//		else
//		{
//			/* Given buffer can't fit in uint32_t */
//			return (0);
//		}
//	}
//
//	return (value);
//}


/**************************************************************************//**
 * @brief
 *   USART0 RX interrupt service routine.
 *
 * @details
 *   The index gets reset to zero when a special character (CR) is received or
 *   the buffer is filled.
 *
 * @note
 *   The *weak* definition for this method is located in `system_efm32hg.h`.
 *****************************************************************************/
void USART0_RX_IRQHandler(void)
{
	/* "static" so it keeps its value between invocations */
	static uint32_t i = 0;

	/* Get and clear the pending USART interrupt flags */
	uint32_t flags = USART_IntGet(dbpointer);
	USART_IntClear(dbpointer, flags);

	/* Store incoming data into dbprint_rx_buffer */
	rx_buffer[i++] = USART_Rx(dbpointer);

	/* Set dbprint_rxdata when a special character is received (~ full line received) */
	if ( (rx_buffer[i - 1] == '\r') || (rx_buffer[i - 1] == '\f') )
	{
		dataReceived = true;
		rx_buffer[i - 1] = '\0'; /* Overwrite CR or LF character */
		i = 0;
	}

	/* Set dbprint_rxdata when the buffer is full */
	if (i >= (DBPRINT_BUFFER_SIZE - 2))
	{
		dataReceived = true;
		rx_buffer[i] = '\0'; /* Do not overwrite last character */
		i = 0;
	}
}


/**************************************************************************//**
 * @brief
 *   USART0 TX interrupt service routine.
 *
 * @details
 *   The index gets reset to zero when all the characters in the buffer are send.
 *
 * @note
 *   The *weak* definition for this method is located in `system_efm32hg.h`.
 *****************************************************************************/
void USART0_TX_IRQHandler(void)
{
	/* "static" so it keeps its value between invocations */
	static uint32_t i = 0;

	/* Get and clear the pending USART interrupt flags */
	uint32_t flags = USART_IntGet(dbpointer);
	USART_IntClear(dbpointer, flags);

	/* Mask flags AND "TX Complete Interrupt Flag" */
	if (flags & USART_IF_TXC)
	{
		/* Index is smaller than the maximum buffer size and
		 * the current item to print is not "NULL" (\0) */
		if ( (i < DBPRINT_BUFFER_SIZE) && (tx_buffer[i] != '\0') )
		{
			/* Transmit byte at current index and increment index */
			USART_Tx(dbpointer, tx_buffer[i++]);
		}
		else
		{
			i = 0; /* No more data to send */
		}
	}
}


/**************************************************************************//**
 * @brief
 *   USART1 RX interrupt service routine.
 *
 * @note
 *   The *weak* definition for this method is located in `system_efm32hg.h`.
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
	/* Call other handler */
	USART0_RX_IRQHandler();
}


/**************************************************************************//**
 * @brief
 *   USART1 TX interrupt service routine.
 *
 * @note
 *   The *weak* definition for this method is located in `system_efm32hg.h`.
 *****************************************************************************/
void USART1_TX_IRQHandler(void)
{
	/* Call other handler */
	USART0_TX_IRQHandler();
}

#endif /* DEBUG_DBPRINT */
