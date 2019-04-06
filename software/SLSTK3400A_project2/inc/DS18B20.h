/***************************************************************************//**
 * @file DS18B20.h
 * @brief All code for the DS18B20 temperature sensor.
 * @version 1.4
 * @author
 *   Alec Vanderhaegen & Sarah Goossens
 *   Modified by Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DS18B20_H_
#define _DS18B20_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/* Maximum waiting value before a reset becomes "failed" */
#define MAX_TIME_CTR 2000


/* Prototypes for methods available to be used elsewhere */
float readTempDS18B20 (void);
void initVDD_DS18B20 (void);
void powerDS18B20 (bool enabled);
bool init_DS18B20 (void);


#endif /* _DS18B20_H_ */