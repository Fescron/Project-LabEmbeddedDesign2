/***************************************************************************//**
 * @file util.h
 * @brief Utility functions.
 * @version 1.2
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _UTIL_H_
#define _UTIL_H_


#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_gpio.h"   /* General Purpose IO */

#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h" 	/* Enable or disable printing to UART */


/* Prototypes */
void initLED (void);
void led (bool enabled);
void Error (uint8_t number);
void Delay (uint32_t dlyTicks);
void systickInterrupts (bool enabled);


#endif /* _UTIL_H_ */
