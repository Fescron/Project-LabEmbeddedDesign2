/***************************************************************************//**
 * @file util.h
 * @brief Utility functions.
 * @version 2.1
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _UTIL_H_
#define _UTIL_H_


#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_cmu.h"    /* Clock Management Unit */
#include "em_gpio.h"   /* General Purpose IO */

#include "../inc/delay.h"     	/* Delay functionality */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h" 	/* Enable or disable printing to UART */


/* Prototypes */
void led (bool enabled);
void error (uint8_t number);


#endif /* _UTIL_H_ */
