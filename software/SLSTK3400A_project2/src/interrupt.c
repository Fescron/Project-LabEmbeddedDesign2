/***************************************************************************//**
 * @file interrupt.c
 * @brief Interrupt functionality.
 * @version 2.1
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Started from https://github.com/Fescron/Project-LabEmbeddedDesign1
 *   @li v1.1: Added dbprintln(""); above dbinfo statements in IRQ handlers to fix
 *             overwriting of text.
 *   @li v1.2: Disabled the RTC counter if GPIO handlers are called, only added necessary includes
 *             in header file, moved the others to the source file, updated documentation.
 *   @li v1.3: Started using set method for the static variable `ADXL_triggered`, added GPIO
 *             wakeup initialization method here, renamed file.
 *   @li v1.4: Stopped disabling the GPIO clock.
 *   @li v1.5: Started using getters/setters to indicate an interrupt to `main.c`.
 *   @li v1.6: Moved IRQ handler of RTC to this `delay.c`.
 *   @li v1.7: Updated clear pending interrupt logic.
 *   @li v1.8: Updated code with new DEFINE checks.
 *   @li v1.9: Removed error calls for "unknown" pins and added flag check for custom Happy Gecko board pinout.
 *   @li v2.0: Stopped disabling the RTC counter on pin interrupts.
 *   @li v2.1: Disabled the RTC counter on a button push because it confused delays called in LoRaWAN code.
 *
 * ******************************************************************************
 *
 * @note
 *   Other interrupt handlers can be found in `delay.c` and `adc.c`.
 *
 * ******************************************************************************
 *
 * @section License
 *
 *   Some methods use code obtained from examples from [Silicon Labs' GitHub](https://github.com/SiliconLabs/peripheral_examples).
 *   These sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


#include <stdint.h>      /* (u)intXX_t */
#include <stdbool.h>     /* "bool", "true", "false" */
#include "em_device.h"   /* Include necessary MCU-specific header file */
#include "em_cmu.h"      /* Clock management unit */
#include "em_gpio.h"     /* General Purpose IO */
#include "em_rtc.h"      /* Real Time Counter (RTC) */

#include "interrupt.h"   /* Corresponding header file */
#include "pin_mapping.h" /* PORT and PIN definitions */
#include "debugging.h"   /* Enable or disable printing to UART */
#include "util.h"     	 /* Utility functionality */
#include "ADXL362.h"     /* Functions related to the accelerometer */


/* Local variables */
/* Volatile because they're modified by an interrupt service routine */
static volatile bool PB0_triggered = false;
static volatile bool PB1_triggered = false;


/**************************************************************************//**
 * @brief
 *   Initialize GPIO wake-up functionality.
 *
 * @details
 *   Initialize buttons `PB0` and `PB1` on falling-edge interrupts and
 *   `ADXL_INT1` on rising-edge interrupts.
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


	/* Clear all odd pin interrupt flags (just in case)
	 * NVIC_ClearPendingIRQ(GPIO_ODD_IRQn); would also work but is less "readable" */
	GPIO_IntClear(0xAAAA);

	/* Clear all even pin interrupt flags (just in case)
	 * NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn); would also work but is less "readable" */
	GPIO_IntClear(0x5555);

	/* All pending interrupts can be cleared with GPIO_IntClear(0xFFFF); */

	/* Enable IRQ for even numbered GPIO pins */
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

	/* Enable IRQ for odd numbered GPIO pins */
	NVIC_EnableIRQ(GPIO_ODD_IRQn);

	/* Enable falling-edge interrupts for PB pins */
	GPIO_ExtIntConfig(PB0_PORT, PB0_PIN, PB0_PIN, false, true, true);
	GPIO_ExtIntConfig(PB1_PORT, PB1_PIN, PB1_PIN, false, true, true);

	/* Enable rising-edge interrupts for ADXL_INT1 */
	GPIO_ExtIntConfig(ADXL_INT1_PORT, ADXL_INT1_PIN, ADXL_INT1_PIN, true, false, true);

#if DEBUGGING == 1 /* DEBUGGING */
	dbinfo("GPIO wake-up initialized");
#endif /* DEBUGGING */

}


/**************************************************************************//**
 * @brief
 *   Getter for the `PB0_triggered` and `PB1_triggered` static variables.
 *
 * @param[in] number
 *   @li `0` - `PB0_triggered` selected.
 *   @li `1` - `PB1_triggered` selected.
 *
 * @return
 *   The value of `PB0_triggered` or `PB1_triggered`.
 *****************************************************************************/
bool BTN_getTriggered (uint8_t number)
{
	if (number == 0) return (PB0_triggered);
	else if (number == 1) return (PB1_triggered);
	else
	{

#if DEBUGGING == 1 /* DEBUGGING */
		dbcrit("Non-existing button selected!");
#endif /* DEBUGGING */

		error(9);

		return (false);
	}
}


/**************************************************************************//**
 * @brief
 *   Setter for the `PB0_triggered` and `PB1_triggered` static variable.
 *
 * @param[in] number
 *   @li `0` - `PB0_triggered selected`.
 *   @li `1` - `PB1_triggered selected`.
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

#if DEBUGGING == 1 /* DEBUGGING */
		dbcrit("Non-existing button selected!");
#endif /* DEBUGGING */

		error(8);
	}
}


/**************************************************************************//**
 * @brief
 *   GPIO Even IRQ for pushbuttons on even-numbered pins.
 *
 * @details
 *   The RTC is also disabled on a button press (*manual wakeup*)
 *
 * @note
 *   The *weak* definition for this method is located in `system_efm32hg.h`.
 *****************************************************************************/
void GPIO_EVEN_IRQHandler (void)
{
	/* Read interrupt flags */
	uint32_t flags = GPIO_IntGet();

	/* Check if PB1 is pushed */
	if (flags == 0x400)
	{
		/* Disable the counter (manual wakeup) */
		RTC_Enable(false);

		PB1_triggered = true;
	}

	/* Clear all even pin interrupt flags */
	GPIO_IntClear(0x5555);
}


/**************************************************************************//**
 * @brief
 *   GPIO Odd IRQ for pushbuttons on odd-numbered pins.
 *
 * @details
 *   The RTC is also disabled on a button press (*manual wakeup*)
 *
 * @note
 *   The *weak* definition for this method is located in `system_efm32hg.h`.
 *****************************************************************************/
void GPIO_ODD_IRQHandler (void)
{
	/* Read interrupt flags */
	uint32_t flags = GPIO_IntGet();

	/* Check if PB0 is pushed */
	if (flags == 0x200)
	{
		/* Disable the counter (manual wakeup) */
		RTC_Enable(false);

		PB0_triggered = true;
	}

	/* Check if INT1 is triggered */
#if CUSTOM_BOARD == 1 /* Custom Happy Gecko pinout */
	if (flags == 0x8) ADXL_setTriggered(true);
#else /* Regular Happy Gecko pinout */
	if (flags == 0x80) ADXL_setTriggered(true);
#endif /* Board pinout selection */

	/* Clear all odd pin interrupt flags */
	GPIO_IntClear(0xAAAA);
}
