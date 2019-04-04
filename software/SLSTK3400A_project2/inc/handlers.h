/***************************************************************************//**
 * @file handlers.h
 * @brief Interrupt handlers.
 * @version 1.1
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _HANDLERS_H_
#define _HANDLERS_H_


#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_gpio.h"   /* General Purpose IO */
#include "em_rtc.h"    /* Real Time Counter (RTC) */

#include "../inc/debugging.h" /* Enable or disable printing to UART */


/* Global variables (project-wide accessible) */
extern volatile bool triggered; /* Accelerometer triggered interrupt */


#endif /* _HANDLERS_H_ */
