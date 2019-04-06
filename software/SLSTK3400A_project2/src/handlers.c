/***************************************************************************//**
 * @file handlers.c
 * @brief
 *   Interrupt handlers for the RTC and GPIO wakeup functionality.
 *   Another interrupt handler can be found in "delay.c".
 * @version 1.2
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   v1.0: Started from https://github.com/Fescron/Project-LabEmbeddedDesign1
 *   v1.1: Added dbprintln(""); above dbinfo statements in IRQ handlers to fix
 *         overwriting of text.
 *   v1.2: Disabled the RTC counter if GPIO handlers are called, only added necessary includes
 *         in header file, moved the others to the source file, updated documentation.
 *
 *   TODO: Make "triggered" static using getter?
 *           => Add triggerd to ADXL362.c, use getters and setters to modify them
 *              from main.c or handlers.c (more "readable" in main.c)
 *         Remove those UART calls, state machine in main?
 *         Remove stdbool include?
 *
 ******************************************************************************/


/* Includes necessary for this source file */
#include <stdint.h>    /* (u)intXX_t */
//#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_gpio.h"   /* General Purpose IO */
#include "em_rtc.h"    /* Real Time Counter (RTC) */

#include "../inc/handlers.h"  /* Corresponding header file */
#include "../inc/debugging.h" /* Enable or disable printing to UART */


/* Global variable (project-wide accessible) */
volatile bool triggered = false; /* TODO: make static using getter? Accelerometer triggered interrupt */


/**************************************************************************//**
 * @brief
 *   RTCC interrupt service routine.
 *
 * @note
 *   The "weak" definition for this method is located in "system_efm32hg.h".
 *****************************************************************************/
void RTC_IRQHandler (void)
{
	/* Disable the counter */
	RTC_Enable(false);

	/* Clear the interrupt source */
	RTC_IntClear(RTC_IFC_COMP0);
}


/**************************************************************************//**
 * @brief
 *   GPIO Even IRQ for pushbuttons on even-numbered pins.
 *
 * @note
 *   The "weak" definition for this method is located in "system_efm32hg.h".
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
	/* Disable the counter */
	RTC_Enable(false);

	/* Read interrupt flags */
	uint32_t flags = GPIO_IntGet();

#ifdef DEBUGGING /* DEBUGGING */
	dbprintln("");
	dbinfo("Even numbered GPIO interrupt triggered.");
	if (flags == 0x400) dbprint_color("PB1\n\r", 4);
#endif /* DEBUGGING */

	/* Clear all even pin interrupt flags */
	GPIO_IntClear(0x5555);
}


/**************************************************************************//**
 * @brief
 *   GPIO Odd IRQ for pushbuttons on odd-numbered pins.
 *
 * @note
 *   The "weak" definition for this method is located in "system_efm32hg.h".
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
	/* Disable the counter */
	RTC_Enable(false);

	/* Read interrupt flags */
	uint32_t flags = GPIO_IntGet();

#ifdef DEBUGGING /* DEBUGGING */
	dbprintln("");
	dbinfo("Odd numbered GPIO interrupt triggered.");
	if (flags == 0x200) dbprint_color("PB0\n\r", 4);
	else if (flags == 0x80) dbprint_color("INT1-PD7\n\r", 4);
#endif /* DEBUGGING */

	/* Indicate that the accelerometer has given an interrupt */
	if (flags == 0x80) triggered = true;

	/* Clear all odd pin interrupt flags */
	GPIO_IntClear(0xAAAA);
}
