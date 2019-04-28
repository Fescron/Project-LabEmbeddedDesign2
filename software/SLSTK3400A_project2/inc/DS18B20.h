/***************************************************************************//**
 * @file DS18B20.h
 * @brief All code for the DS18B20 temperature sensor.
 * @version 2.0
 * @author
 *   Alec Vanderhaegen & Sarah Goossens@n
 *   Modified by Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DS18B20_H_
#define _DS18B20_H_


/* Public definition */
/** Maximum waiting value before a reset becomes *failed* */
#define MAX_TIME_CTR 2000


/* Public prototype */
float readTempDS18B20 (void);


#endif /* _DS18B20_H_ */
