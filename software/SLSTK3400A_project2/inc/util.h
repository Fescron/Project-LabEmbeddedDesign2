/***************************************************************************//**
 * @file util.h
 * @brief Utility functions.
 * @version 2.2
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _UTIL_H_
#define _UTIL_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/* Prototypes for methods available to be used elsewhere */
void led (bool enabled);
void error (uint8_t number);


#endif /* _UTIL_H_ */
