/***************************************************************************//**
 * @file adc.h
 * @brief ADC functionality for reading the (battery) voltage and internal temperature.
 * @version 1.2
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _ADC_H_
#define _ADC_H_


/* Includes necessary for this header file */
#include <stdint.h>    /* (u)intXX_t */


/** Enum type for the ADC */
typedef enum adc_measurements
{
	BATTERY_VOLTAGE,
	INTERNAL_TEMPERATURE
} ADC_Measurement_t;


/* Public prototypes */
void initADC (ADC_Measurement_t peripheral);
uint32_t readADC (ADC_Measurement_t peripheral);


#endif /* _ADC_H_ */
