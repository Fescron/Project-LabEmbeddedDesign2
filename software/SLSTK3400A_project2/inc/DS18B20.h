/***************************************************************************//**
 * @file DS18B20.h
 * @brief All code for the DS18B20 temperature sensor.
 * @version 1.2
 * @author
 *   Alec Vanderhaegen & Sarah Goossens
 *   Modified by Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DS18B20_H_
#define _DS18B20_H_


#include <stdint.h>             /* (u)intXX_t */
#include <stdbool.h>            /* "bool", "true", "false" */
#include "em_cmu.h"             /* Clock Management Unit */
#include "em_gpio.h"            /* General Purpose IO (GPIO) peripheral API */
#include "../inc/udelay.h"      /* Microsecond delay routine */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */

#include "../inc/debugging.h"   /* Enable or disable printing to UART */


/* Maximum waiting value before a reset becomes "failed" */
#define MAX_TIME_CTR 2000


/* Prototypes */
float readTempDS18B20 (void);

void initVDD_DS18B20 (void);
void powerDS18B20 (bool enabled);
bool init_DS18B20 (void);

void writeByteToDS18B20 (uint8_t data);
uint8_t readByteFromDS18B20 (void);
float convertTempData (uint8_t temp1, uint8_t temp2);


#endif /* _DS18B20_H_ */
