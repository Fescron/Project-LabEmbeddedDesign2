/***************************************************************************//**
 * @file interrupt.c
 * @brief Interrupt functionality.
 * @version 1.6
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
 *   v1.5: Started using getters/setters to indicate an interrupt to "main.c".
 *   v1.6: Moved IRQ handler of RTC to this "delay.c".
 *
 *   TODO: Check if clear pending interrupts is necessary?
 *         GPIO_IntClear(0xFFFF); vs NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);?
 *
 * ******************************************************************************
 *
 * @note
 *   Other interrupt handlers can be found in "delay.c" and "other.c".
 *
 ******************************************************************************/


#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_cmu.h"    /* Clock management unit */
#include "em_gpio.h"   /* General Purpose IO */
#include "em_rtc.h"    /* Real Time Counter (RTC) */

#include "../inc/interrupt.h"   /* Corresponding header file */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h"   /* Enable or disable printing to UART */
#include "../inc/util.h"     	/* Utility functionality */
#include "../inc/ADXL362.h"     /* Functions related to the accelerometer */


/** Local variables */
/* Volatile because they're modified by an interrupt service routine */
static volatile bool PB0_triggered = false;
static volatile bool PB1_triggered = false;


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
	// Or with:
	//NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	//NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);

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
 *   Getter for the "PB0_triggered" and "PB1_triggered" static variables.
 *
 * @param[in] number
 *   @li 0 - PB0_triggered selected.
 *   @li 1 - PB1_triggered selected.
 *
 * @return
 *   The value of "PB0_triggered" or "PB1_triggered".
 *****************************************************************************/
bool BTN_getTriggered (uint8_t number)
{
	if (number == 0) return (PB0_triggered);
	else if (number == 1) return (PB1_triggered);
	else
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Non-existing button selected!");
#endif /* DEBUGGING */

		error(9);

		return (false);
	}
}


/**************************************************************************//**
 * @brief
 *   Setter for the "PB0_triggered" and "PB1_triggered" static variable.
 *
 * @param[in] number
 *   @li 0 - PB0_triggered selected.
 *   @li 1 - PB1_triggered selected.
 *
 * @param[in] value
 *   The boolean value to set to the selected static variable.
 *****************************************************************************/
void BTN_setTriggered (uint8_t number, bool value)
{
	if (number == 0) PB0_triggered = value;
	else if (number == 1) PB1_triggered = value;
	else
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Non-existing button selected!");
#endif /* DEBUGGING */

		error(8);
	}
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

	/* Check if PB1 is pushed */
	if (flags == 0x400) PB1_triggered = true;
	else
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Unknown even-numbered IRQ pin triggered!");
#endif /* DEBUGGING */

		error(2);
	}

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

	/* Check if PB0 or INT1-PD7 is pushed */
	if (flags == 0x200) PB0_triggered = true;
	else if (flags == 0x80) ADXL_setTriggered(true);
	else
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Unknown odd-numbered IRQ pin triggered!");
#endif /* DEBUGGING */

		error(3);
	}

	/* Clear all odd pin interrupt flags */
	GPIO_IntClear(0xAAAA);
}
