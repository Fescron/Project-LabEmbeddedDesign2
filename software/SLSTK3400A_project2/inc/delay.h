/***************************************************************************//**
 * @file delay.h
 * @brief Delay functions.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DELAY_H_
#define _DELAY_H_


#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_cmu.h"    /* Clock management unit */
#include "em_emu.h"    /* Energy Management Unit */
#include "em_gpio.h"   /* General Purpose IO */
#include "em_rtc.h"    /* Real Time Counter (RTC) */

#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h" 	/* Enable or disable printing to UART */


/* Definitions for RTC compare interrupts */
#define LFXOFREQ 32768
#define LFXOFREQ_MS 32.768


/* Prototypes */
void initRTCcomp (void);
void delayRTCC_EM1 (uint32_t msDelay);
void delayRTCC_EM2 (uint32_t msDelay);
void sleepRTCC_EM2 (uint32_t sleep);
void Delay (uint32_t dlyTicks);
void systickInterrupts (bool enabled);


#endif /* _DELAY_H_ */
