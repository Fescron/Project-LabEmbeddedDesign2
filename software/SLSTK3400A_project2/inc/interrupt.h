/***************************************************************************//**
 * @file interrupt.h
 * @brief Interrupt functionality.
 * @version 2.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/* Public prototypes */
void initGPIOwakeup (void);
bool BTN_getTriggered (uint8_t number);
void BTN_setTriggered (uint8_t number, bool value);


#endif /* _INTERRUPT_H_ */
