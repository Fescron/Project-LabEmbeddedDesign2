/***************************************************************************//**
 * @file DS18B20.h
 * @brief All code for the DS18B20 temperature sensor.
 * @version 1.6
 * @author
 *   Alec Vanderhaegen & Sarah Goossens
 *   Modified by Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DS18B20_H_
#define _DS18B20_H_


/** Include necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */


/** Public definitions */
#define MAX_TIME_CTR 2000 /* Maximum waiting value before a reset becomes "failed" */


/** Public prototype */
float readTempDS18B20 (void);


#endif /* _DS18B20_H_ */
