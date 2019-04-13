/***************************************************************************//**
 * @file adc.h
 * @brief ADC functionality for reading the (battery) voltage and internal temperature.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _ADC_H_
#define _ADC_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/* Public prototypes */
void initADC (bool temperature);
uint32_t readADC (bool temperature);


#endif /* _ADC_H_ */
