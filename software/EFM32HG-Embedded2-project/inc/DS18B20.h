/***************************************************************************//**
 * @file DS18B20.h
 * @brief All code for the DS18B20 temperature sensor.
 * @version 2.2
 * @author
 *   Alec Vanderhaegen & Sarah Goossens@n
 *   Modified by Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DS18B20_H_
#define _DS18B20_H_


/* Include necessary for this header file */
#include <stdint.h> /* (u)intXX_t */


/* Public prototype */
uint32_t readTempDS18B20 (void);


#endif /* _DS18B20_H_ */