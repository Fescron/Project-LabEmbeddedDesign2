/***************************************************************************//**
 * @file util.c
 * @brief Utility functions.
 * @version 1.2
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   v1.0: Start with the code from https://github.com/Fescron/Project-LabEmbeddedDesign1/tree/master/code/SLSTK3400A_ADXL362
 *   v1.1: Change PinModeSet DOUT value to 0 in initLED.
 *   v1.2: Remove unnecessary "GPIO_PinOutClear" line in initLED.
 *
 ******************************************************************************/


#include "../inc/util.h"


/* Global variables */
volatile uint32_t msTicks; /* Volatile because it's a global variable that's modified by an interrupt service routine */


/**************************************************************************//**
 * @brief
 *   Initialize the LED.
 *****************************************************************************/
void initLED (void)
{
	/* In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1".
	 * This means that "GPIO_PinOutClear(...)" is not necessary after this mode change.*/
	GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, 0);
}


/**************************************************************************//**
 * @brief
 *   Enable or disable the LED.
 *
 * @param[in] enabled
 *   @li True - Enable LED
 *   @li False - Disable LED.
 *****************************************************************************/
void led (bool enabled)
{
	if (enabled) GPIO_PinOutSet(LED_PORT, LED_PIN);
	else GPIO_PinOutClear(LED_PORT, LED_PIN);
}


/**************************************************************************//**
 * @brief
 *   Error method.
 *
 * @details
 *   Flashes the LED, displays a UART message and holds
 *   the microcontroller forever in a loop until it gets reset.
 *
 * @param[in] number
 *   The number to indicate where in the code the error was thrown.
 *****************************************************************************/
void Error (uint8_t number)
{

#ifdef DEBUGGING /* DEBUGGING */
	dbcritInt(">>> Error (", number, ")! Please reset MCU. <<<");
#endif /* DEBUGGING */

	while(1)
	{
		Delay(100);
		GPIO_PinOutToggle(LED_PORT, LED_PIN); /* Toggle LED */
	}
}


/**************************************************************************//**
 * @brief
 *   Interrupt Service Routine for system tick counter.
 *****************************************************************************/
void SysTick_Handler (void)
{
	msTicks++; /* Increment counter necessary in Delay() */
}


/**************************************************************************//**
 * @brief
 *   Waits a certain amount of milliseconds using the systicks.
 *
 * @param[in] dlyTicks
 *   Number of milliseconds (ticks) to wait.
 *****************************************************************************/
void Delay (uint32_t dlyTicks)
{
	/* TODO: Maybe enter EM1 of 2? */
	// EMU_EnterEM1();

	uint32_t curTicks = msTicks;

	while ((msTicks - curTicks) < dlyTicks);
}


/**************************************************************************//**
 * @brief
 *   Disable
 *
 * @note
 *   SysTick interrupt and counter (used by Delay) need to
 *   be disabled before going to EM2.
 *
 * @param[in] enabled
 *   @li True - Enable SysTick interrupt and counter by setting their bits.
 *   @li False - Disable SysTick interrupt and counter by clearing their bits.
 *****************************************************************************/
void systickInterrupts (bool enabled)
{
	if (enabled) SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	else SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk & ~SysTick_CTRL_ENABLE_Msk;
}

