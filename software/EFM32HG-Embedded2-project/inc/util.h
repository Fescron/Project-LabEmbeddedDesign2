/***************************************************************************//**
 * @file util.h
 * @brief Utility functionality.
 * @version 3.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _UTIL_H_
#define _UTIL_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/** Public definition to enable/disable the logic to send certain error call values (everything except 30 - 50) to the cloud using LoRaWAN.
 *    @li `0` - Keep the MCU in a `while(true)` loop if the `error` method is called while flashing the LED and displaying a UART message.
 *    @li `1` - Display a UART message when the `error` method is called but also forward the number to the cloud using LoRaWAN. Don't go in a `while(true)` loop.  */
#define ERROR_FORWARDING 1


/* Public prototypes */
void led (bool enabled);
void error (uint8_t number);


#endif /* _UTIL_H_ */
