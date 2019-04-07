/***************************************************************************//**
 * @file other.c
 * @brief Cable checking and battery voltage functionality.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _OTHER_H_
#define _OTHER_H_


/** Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/** Public prototypes */
bool checkCable (void);
void initVBAT (void);
uint32_t readVBAT (void);


#endif /* _OTHER_H_ */
