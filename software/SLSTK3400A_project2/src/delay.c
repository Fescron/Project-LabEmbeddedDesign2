/***************************************************************************//**
 * @file delay.c
 * @brief Utility functions.
 * @version 1.3
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
 *
 *   TODO: Enable disable/enable clock functionality? (slows down the logic a lot last time tested...)
 *         RTC calendar (RTCC =/= RTCcompare?) could perhaps be better to wake the MCU every hour?
 *           => Possible in EM3 when using an external crystal?
 *         Make initRTCcomp static?
 *         Check the sections about Energy monitor and Crystals and RC oscillators
 *           => DRAMCO has perhaps chosen the crystal because this was more stable for
 *              high frequency (baudrate) communication. (leuart RNxxx...)
 *
 * ******************************************************************************
 *
 * @section Energy monitor and energy modes
 *
 *   When in "debug" mode, the MCU doesn't really go lower than <TODO: check this> EM1. Use the
 *   energy profiler and manually reset the MCU once for it to go in the right energy modes.
 *
 *   At one point a method was developed to go in EM1 when waiting in a delay.
 *   However this didn't seem to work as intended and EM2 would also be fine.
 *   Because of this, development for this EM1 delay method was halted.
 *   EM1 is sometimes used when waiting on bits to be set.
 *
 *   When the MCU is in EM3, it can normally only be woken up using a pin
 *   change interrupt, not using the RTC.
 *
 * ******************************************************************************
 *
 * @section Crystals and RC oscillators
 *
 *   Apparently it's more energy efficient to use an external oscillator/crystal
 *   instead of the internal one. They only reason to use an internal one could be
 *   to reduce the part count. At one point I tried to use the Ultra low-frequency
 *   RC oscillator (ULFRCO) based on an example from SiliconLabs's GitHub (rtc_ulfrco),
 *   but development was halted shortly after this finding.
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
static bool SysTick_init = false;
static bool RTCC_init = false;


/**************************************************************************//**
 * @brief
 *   Wait for a certain amount of milliseconds.
 *
 * @details
 *   The type of delay (using SysTick or RTCcompare) can be selected by
 *   (un)commenting "#define SYSTICKDELAY" in "delay.h"
 *
 * @param[in] msDelay
 *   The delay time in milliseconds.
 *****************************************************************************/
void delay (uint32_t msDelay)
{

#ifdef SYSTICKDELAY /* SysTick delay selected */

	/* Initialize SysTick if not already the case */
	if (!SysTick_init)
	{
		/* Initialize and start SysTick
		 * Number of ticks between interrupt = cmuClock_CORE/1000 */
		if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("SysTick initialized");
#endif /* DEBUGGING */

		SysTick_init = true;
	}
	else
	{
		/* Enable SysTick interrupt and counter by setting their bits. */
		SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	}

#ifdef DEBUGGING /* DEBUGGING */
	dbwarnInt("Waiting for ", msDelay, " ms\n\r");
#endif /* DEBUGGING */

	/* Wait a certain amount of ticks */
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < msDelay);

	/* Disable SysTick interrupt and counter (needs to be done before entering EM2) by clearing their bits. */
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk & ~SysTick_CTRL_ENABLE_Msk;

#else /* EM2 RTCC delay selected */

	/* Initialize RTCC if not already the case */
	if (!RTCC_init) initRTCcomp();

#ifdef DEBUGGING /* DEBUGGING */
	dbwarnInt("Waiting in EM2 for ", msDelay, " ms\n\r");
#endif /* DEBUGGING */

	/* Set RTC compare value for RTC compare register 0 */
	RTC_CompareSet(0, (LFXOFREQ_MS * msDelay)); /* <= 0x00ffffff = 16777215 */

	/* Start the RTC */
	RTC_Enable(true);

	/* Enter EM2 */
	EMU_EnterEM2(true); /* "true" doesn't seem to have any effect (save and restore oscillators, clocks and voltage scaling) */

	/* Disable used oscillator and clocks after wakeup */
//	CMU_OscillatorEnable(cmuOsc_LFXO, false, true);
//	CMU_ClockEnable(cmuClock_HFLE, false);
//	CMU_ClockEnable(cmuClock_RTC, false);

#endif

}


/**************************************************************************//**
 * @brief
 *   RTCC initialization
 *
 * @details
 *   The RTC compare functionality uses the low-frequency crystal oscillator
 *   (LFXO) as the source.
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

	RTCC_init = true;
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
	/* Initialize RTCC if not already the case */
	if (!RTCC_init){
		initRTCcomp();
	}
	else
	{
		/* Enable necessary oscillator and clocks */
//		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
//		CMU_ClockEnable(cmuClock_HFLE, true);
//		CMU_ClockEnable(cmuClock_RTC, true);
	}

#ifdef DEBUGGING /* DEBUGGING */
	dbwarnInt("Sleeping in EM2 for ", sSleep, "s\n\r");
#endif /* DEBUGGING */

	/* Set RTC compare value for RTC compare register 0 */
	RTC_CompareSet(0, (LFXOFREQ * sSleep)); /* <= 0x00ffffff = 16777215 */

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
 *   Interrupt Service Routine for system tick counter.
 *****************************************************************************/
void SysTick_Handler (void)
{
	msTicks++; /* Increment counter necessary by SysTick delay functionality */
}
