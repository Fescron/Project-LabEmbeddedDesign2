/***************************************************************************//**
 * @file delay.c
 * @brief Delay functionality.
 * @version 1.6
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   v1.0: Moved delay functionality from "util.c" to this file.
 *   v1.1: Changed global variables to be static (~hidden) and added dbprint "\n\r" fixes.
 *   v1.2: Removed EM1 delay method (see note in delayRTCC_EM2).
 *   v1.3: Cleaned up includes, added option to select SysTick/EM2 delay functionality
 *         and moved all of the logic in one "delay" method. Renamed sleep method.
 *   v1.4: Changed names of static variables, made initRTCcomp static.
 *   v1.5: Updated documentation.
 *   v1.6: Changed RTCcomp names to RTC
 *
 *   TODO: Remove stdint include?
 *         Enable disable/enable clock functionality? (slows down the logic a lot last time tested...)
 *         Add checks if delay fits in field?
 *         Check if HFLE needs to be enabled.
 *         Use cmuSelect_ULFRCO?
 *
 ******************************************************************************/


/* Includes necessary for this source file */
//#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_cmu.h"    /* Clock management unit */
#include "em_emu.h"    /* Energy Management Unit */
#include "em_gpio.h"   /* General Purpose IO */
#include "em_rtc.h"    /* Real Time Counter (RTC) */

#include "../inc/delay.h"       /* Corresponding header file */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h" 	/* Enable or disable printing to UART */


/* Static variables only available and used in this file */
static volatile uint32_t msTicks; /* Volatile because it's a global variable that's modified by an interrupt service routine */
static bool RTC_initialized = false;

#ifdef SYSTICKDELAY /* SysTick delay selected */
static bool SysTick_initialized = false;
#endif /* SysTick/RTC selection */


/* Prototype for static method only used by other methods in this file
 * (Not available to be used elsewhere) */
static void initRTC (void);


/**************************************************************************//**
 * @brief
 *   Wait for a certain amount of milliseconds.
 *
 * @details
 *   The type of delay (using SysTick or the RTC compare functionality) can be
 *   selected by (un)commenting "#define SYSTICKDELAY" in "delay.h"
 *   This method also initializes SysTick/RTC if necessary.
 *
 * @param[in] msDelay
 *   The delay time in milliseconds.
 *****************************************************************************/
void delay (uint32_t msDelay)
{

#ifdef SYSTICKDELAY /* SysTick delay selected */

	/* Initialize SysTick if not already the case */
	if (!SysTick_initialized)
	{
		/* Initialize and start SysTick
		 * Number of ticks between interrupt = cmuClock_CORE/1000 */
		if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("SysTick initialized");
#endif /* DEBUGGING */

		SysTick_initialized = true;
	}
	else
	{
		/* Enable SysTick interrupt and counter by setting their bits. */
		SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	}

	/* Wait a certain amount of ticks */
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < msDelay);

	/* Disable SysTick interrupt and counter (needs to be done before entering EM2) by clearing their bits. */
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk & ~SysTick_CTRL_ENABLE_Msk;

#else /* EM2 RTC delay selected */

	/* Initialize RTC if not already the case */
	if (!RTC_initialized) initRTC();

	/* Set RTC compare value for RTC compare register 0 */
	RTC_CompareSet(0, (LFXOFREQ_MS * msDelay)); /* <= 0x00ffffff = 16777215 TODO: add check if it fits */

	/* Start the RTC */
	RTC_Enable(true);

	/* Enter EM2 */
	EMU_EnterEM2(true); /* "true" doesn't seem to have any effect (save and restore oscillators, clocks and voltage scaling) */

	/* Disable used oscillator and clocks after wakeup */
//	CMU_OscillatorEnable(cmuOsc_LFXO, false, true);
//	CMU_ClockEnable(cmuClock_HFLE, false);
//	CMU_ClockEnable(cmuClock_RTC, false);

#endif /* SysTick/RTC selection */

}


/**************************************************************************//**
 * @brief
 *   Sleep for a certain amount of seconds in EM2.
 *
 * @param[in] sSleep
 *   The sleep time in seconds.
 *****************************************************************************/
void sleep (uint32_t sSleep)
{
	/* Initialize RTC if not already the case */
	if (!RTC_initialized) initRTC();
	else
	{
		/* Enable necessary oscillator and clocks */
//		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
//		CMU_ClockEnable(cmuClock_HFLE, true);
//		CMU_ClockEnable(cmuClock_RTC, true);
	}

#ifdef DEBUGGING /* DEBUGGING */
	dbwarnInt("Sleeping in EM2 for ", sSleep, " s");
#endif /* DEBUGGING */

	/* Set RTC compare value for RTC compare register 0 */
	RTC_CompareSet(0, (LFXOFREQ * sSleep)); /* <= 0x00ffffff = 16777215 TODO: add check if it fits */

	/* Start the RTC */
	RTC_Enable(true);

	/* Enter EM2 */
	EMU_EnterEM2(true); /* "true" doesn't seem to have any effect (save and restore oscillators, clocks and voltage scaling) */

	/* Disable used oscillator and clocks after wakeup */
//	CMU_OscillatorEnable(cmuOsc_LFXO, false, true);
//	CMU_ClockEnable(cmuClock_HFLE, false);
//	CMU_ClockEnable(cmuClock_RTC, false);
}


/**************************************************************************//**
 * @brief
 *   RTC initialization.
 *
 * @details
 *   The RTC (compare) functionality uses the low-frequency crystal oscillator
 *   (LFXO) as the source.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *****************************************************************************/
static void initRTC (void)
{
	/* Enable the low-frequency crystal oscillator for the RTC */
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

	/* Enable the clock to the interface of the low energy modules
	 * cmuClock_CORELE = cmuClock_HFLE (deprecated) */
	CMU_ClockEnable(cmuClock_HFLE, true); /* TODO: check if this is necessary? (emodes.c) */

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
	dbinfo("RTC initialized\n\r");
#endif /* DEBUGGING */

	RTC_initialized = true;
}


/**************************************************************************//**
 * @brief
 *   Interrupt Service Routine for system tick counter.
 *****************************************************************************/
void SysTick_Handler (void)
{
	msTicks++; /* Increment counter necessary by SysTick delay functionality */
}
