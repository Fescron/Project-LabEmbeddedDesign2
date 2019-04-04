/***************************************************************************//**
 * @file handlers.c
 * @brief
 *   Interrupt handlers for the RTC and GPIO wakeup functionality.
 *   More interrupt handlers can be found in "util.c".
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


#include "../inc/handlers.h"


/* Global variables */
volatile bool triggered = false; /* Accelerometer triggered interrupt */


/**************************************************************************//**
 * @brief
 *   RTCC interrupt service routine.
 *
 * @note
 *   The "weak" definition for this method is located in "system_efm32hg.h".
 *****************************************************************************/
void RTC_IRQHandler (void)
{
	/* Reset counter */
	//RTC_CounterReset(); /* TODO: remove if the new delay methods work as intended */

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
	/* Read interrupt flags */
	uint32_t flags = GPIO_IntGet();

#ifdef DEBUGGING /* DEBUGGING */
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
	/* Read interrupt flags */
	uint32_t flags = GPIO_IntGet();

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("Odd numbered GPIO interrupt triggered.");
	if (flags == 0x200) dbprint_color("PB0\n\r", 4);
	else if (flags == 0x80) dbprint_color("INT1-PD7\n\r", 4);
#endif /* DEBUGGING */

	/* Indicate that the accelerometer has given an interrupt */
	if (flags == 0x80) triggered = true;

	/* Clear all odd pin interrupt flags */
	GPIO_IntClear(0xAAAA);
}

