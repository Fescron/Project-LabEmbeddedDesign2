/***************************************************************************//**
 * @file delay.c
 * @brief Utility functions.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   v1.0: Move delay functionality from "util.c" to this file.
 *
 *   TODO: Fix EM1 delay
 *         Disable oscillators and clocks before going to sleep
 *         Add documentation about energy monitor & EM modes
 *
 ******************************************************************************/


#include "../inc/delay.h"


/* Global variables */
volatile uint32_t msTicks; /* Volatile because it's a global variable that's modified by an interrupt service routine */
bool RTCCinitialized = false;


/**************************************************************************//**
 * @brief
 *   RTCC initialization
 *
 * @details
 *   The RTC compare functionality uses the low-frequency crystal oscillator
 *   (LFXO) as the source.
 *
 * @note
 *   Apparently it's more energy efficient to use an external oscillator/crystal
 *   instead of the internal one. They only reason to use an internal one could be
 *   to reduce the part count. At one point I tried to use the Ultra low-frequency
 *   RC oscillator (ULFRCO) based on an example from SiliconLabs's GitHub (rtc_ulfrco),
 *   but development was halted shortly after this finding.
 *****************************************************************************/
void initRTCcomp (void)
{
	/* Enable the low-frequency crystal oscillator for the RTC */
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

	/* Enable the clock to the interface of the low energy modules
	 * cmuClock_CORELE = cmuClock_HFLE (deprecated) */
	CMU_ClockEnable(cmuClock_HFLE, true);

	/* Route the LFXO clock to the RTC */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	/* Turn on the RTC clock */
	CMU_ClockEnable(cmuClock_RTC, true);

	/* Allow channel 0 to cause an interrupt */
	RTC_IntEnable(RTC_IEN_COMP0);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Configure the RTC settings */
	RTC_Init_TypeDef rtc = RTC_INIT_DEFAULT;
	rtc.enable = false; /* Don't start counting when initialization is done */

	/* Initialize RTC with pre-defined settings */
	RTC_Init(&rtc);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("RTCC initialized");
#endif /* DEBUGGING */

	RTCCinitialized = true;
}


/**************************************************************************//**
 * @brief
 *   Generate a delay and wait in EM1.
 *
 * @param[in] msDelay
 *   The delay in milliseconds.
 *****************************************************************************/
void delayRTCC_EM1 (uint32_t msDelay)
{
	/* Initialize RTCC if not already the case */
	if (!RTCCinitialized) initRTCcomp();

#ifdef DEBUGGING /* DEBUGGING */
	dbwarnInt("Waiting in EM1 for ", msDelay, "ms");
#endif /* DEBUGGING */

	/* Set RTC compare value for RTC compare register 0 */
	RTC_CompareSet(0, (LFXOFREQ_MS * msDelay)); /* <= 0x00ffffff */

	/* Start the RTC */
	RTC_Enable(true);

	/* Enter EM1 */
	EMU_EnterEM1(); /* TODO: this doesn't seem to work at all */
}


/**************************************************************************//**
 * @brief
 *   Generate a delay and wait in EM2.
 *
 * @param[in] msDelay
 *   The delay in milliseconds.
 *****************************************************************************/
void delayRTCC_EM2 (uint32_t msDelay)
{
	/* Initialize RTCC if not already the case */
	if (!RTCCinitialized) initRTCcomp();

#ifdef DEBUGGING /* DEBUGGING */
	dbwarnInt("Waiting in EM2 for ", msDelay, "ms");
#endif /* DEBUGGING */

	/* Set RTC compare value for RTC compare register 0 */
	RTC_CompareSet(0, (LFXOFREQ_MS * msDelay)); /* <= 0x00ffffff */

	/* Start the RTC */
	RTC_Enable(true);

	/* Enter EM2 */
	EMU_EnterEM2(true); /* "true" doesn't seem to have any effect (save and restore oscillators, clocks and voltage scaling) */
}


/**************************************************************************//**
 * @brief
 *   Sleep for a certain amount of seconds in EM2.
 *
 * @param[in] sleep
 *   The sleep time in seconds.
 *****************************************************************************/
void sleepRTCC_EM2 (uint32_t sleep)
{
	/* Initialize RTCC if not already the case */
	if (!RTCCinitialized) initRTCcomp();

#ifdef DEBUGGING /* DEBUGGING */
	dbwarnInt("Sleeping in EM2 for ", sleep, "s");
#endif /* DEBUGGING */

	/* Set RTC compare value for RTC compare register 0 */
	RTC_CompareSet(0, (LFXOFREQ * sleep)); /* <= 0x00ffffff */

	/* Start the RTC */
	RTC_Enable(true);

	/* Enter EM2 */
	EMU_EnterEM2(true); /* "true" doesn't seem to have any effect (save and restore oscillators, clocks and voltage scaling) */
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
	/* TODO: Maybe enter EM1 or 2? */
	// EMU_EnterEM1();

	uint32_t curTicks = msTicks;

	while ((msTicks - curTicks) < dlyTicks);
}


/**************************************************************************//**
 * @brief
 *   Enable or disable sysTick interrupt and counter.
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
