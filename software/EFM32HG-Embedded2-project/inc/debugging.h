/***************************************************************************//**
 * @file debugging.h
 * @brief Enable or disable printing to UART.
 * @details
 *   This header file is called in every other file where there are UART
 *   debugging statements. Depending on the value of `DEBUGGING`, UART statements
 *   are enabled or disabled.
 * @version 1.1
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DEBUGGING_H_
#define _DEBUGGING_H_


/** Public definition to enable/disable UART debugging
 *    @li `1` - Enable the UART debugging statements.
 *    @li `0` - Remove all UART debugging statements from the uploaded code. */
#define DEBUGGING 1


#if DEBUGGING == 1 /* DEBUGGING */
#include "dbprint.h"
#endif /* DEBUGGING */


#endif /* _DEBUGGING_H_ */
