/***************************************************************************//**
 * @file ADXL362.h
 * @brief All code for the ADXL362 accelerometer.
 * @version 2.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _ADXL362_H_
#define _ADXL362_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/** Enum type for the measurement range */
typedef enum adxl_range
{
	ADXL_RANGE_2G, /* +- 2g (reset default) */
	ADXL_RANGE_4G, /* +- 4g */
	ADXL_RANGE_8G  /* +- 8g */
} ADXL_Range_t;

/** Enum type for the ODR */
typedef enum adxl_odr
{
	ADXL_ODR_12_5, /* 12.5 Hz */
	ADXL_ODR_25,   /* 25 Hz */
	ADXL_ODR_50,   /* 50 Hz */
	ADXL_ODR_100,  /* 100 Hz (reset default) */
	ADXL_ODR_200,  /* 200 Hz */
	ADXL_ODR_400   /* 400 Hz */
} ADXL_ODR_t;


/* Public prototypes */
void initADXL (void);

void ADXL_setTriggered (bool triggered);
bool ADXL_getTriggered (void);
void ADXL_ackInterrupt (void);

void ADXL_enableSPI (bool enabled);
void ADXL_enableMeasure (bool enabled);

void ADXL_configRange (ADXL_Range_t givenRange);
void ADXL_configODR (ADXL_ODR_t givenODR);
void ADXL_configActivity (uint8_t gThreshold);

void ADXL_readValues (void);

void testADXL (void);


#endif /* _ADXL362_H_ */
