/***************************************************************************//**
 * @file interrupt.c
 * @brief GPIO wakeup initialization method and interrupt handlers.
 * @version 1.4
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
 *   v1.3: Started using set method for the static variable "ADXL_triggered", added GPIO
 *         wakeup initialization method here, renamed file.
 *   v1.4: Stopped disabling the GPIO clock.
 *
 *   TODO: Remove those UART calls, state machine in main?
 *
 * ******************************************************************************
 *
 * @note
 *   Another interrupt handler can be found in "delay.c".
 *
 ******************************************************************************/


/* Includes necessary for this source file */
#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_cmu.h"    /* Clock management unit */
#include "em_gpio.h"   /* General Purpose IO */
#include "em_rtc.h"    /* Real Time Counter (RTC) */

#include "../inc/interrupt.h"   /* Corresponding header file */
#include "../inc/ADXL362.h"     /* Functions related to the accelerometer */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h"   /* Enable or disable printing to UART */


/**************************************************************************//**
 * @brief
 *   Initialize GPIO wakeup functionality.
 *
 * @details
 *   Initialize buttons PB0 and PB1 on falling-edge interrupts and
 *   ADXL_INT1 on rising-edge interrupts.
 *****************************************************************************/
void initGPIOwakeup (void)
{
	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Configure PB0 and PB1 as input with glitch filter enabled, last argument sets pull direction */
	GPIO_PinModeSet(PB0_PORT, PB0_PIN, gpioModeInputPullFilter, 1);
	GPIO_PinModeSet(PB1_PORT, PB1_PIN, gpioModeInputPullFilter, 1);

	/* Configure ADXL_INT1 as input, the last argument enables the filter */
	GPIO_PinModeSet(ADXL_INT1_PORT, ADXL_INT1_PIN, gpioModeInput, 1);

	/* Clear pending interrupts */
	//GPIO_IntClear(0xFFFF);

	/* Enable IRQ for even numbered GPIO pins */
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

	/* Enable IRQ for odd numbered GPIO pins */
	NVIC_EnableIRQ(GPIO_ODD_IRQn);

	/* Enable falling-edge interrupts for PB pins */
	GPIO_ExtIntConfig(PB0_PORT, PB0_PIN, PB0_PIN, false, true, true);
	GPIO_ExtIntConfig(PB1_PORT, PB1_PIN, PB1_PIN, false, true, true);

	/* Enable rising-edge interrupts for ADXL_INT1 */
	GPIO_ExtIntConfig(ADXL_INT1_PORT, ADXL_INT1_PIN, ADXL_INT1_PIN, true, false, true);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("GPIO wakeup initialized");
#endif /* DEBUGGING */

}


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
	if (flags == 0x80) ADXL_setTriggered(true);

	/* Clear all odd pin interrupt flags */
	GPIO_IntClear(0xAAAA);
}
