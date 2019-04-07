/***************************************************************************//**
 * @file ADXL362.h
 * @brief All code for the ADXL362 accelerometer.
 * @version 1.7
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _ADXL362_H_
#define _ADXL362_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/* ADXL362 register definitions */
#define ADXL_REG_DEVID_AD 		0x00 /* Reset: 0xAD */
#define ADXL_REG_DEVID_MST 		0x01 /* Reset: 0x1D */
#define ADXL_REG_PARTID 		0x02 /* Reset: 0xF2 */
#define ADXL_REG_REVID 			0x03 /* Reset: 0x01 (can be incremented) */
#define ADXL_REG_XDATA 			0x08
#define ADXL_REG_YDATA 			0x09
#define ADXL_REG_ZDATA 			0x0A
#define ADXL_REG_STATUS 		0x0B
#define ADXL_REG_TEMP_L 		0x14
#define ADXL_REG_TEMP_H 		0x15
#define ADXL_REG_SOFT_RESET 	0x1F /* Needs to be 0x52 ("R") written to for a soft reset */
#define ADXL_REG_THRESH_ACT_L	0x20 /* 7:0 bits used */
#define ADXL_REG_THRESH_ACT_H	0x21 /* 2:0 bits used */
#define ADXL_REG_ACT_INACT_CTL  0x27 /* Activity/Inactivity control register: XX - XX - LINKLOOP - LINKLOOP - INACT_REF - INACT_EN - ACT_REF - ACT_EN */
#define ADXL_REG_INTMAP1 		0x2A /* INT_LOW -- AWAKE -- INACT -- ACT -- FIFO_OVERRUN -- FIFO_WATERMARK -- FIFO_READY -- DATA_READY */
#define ADXL_REG_INTMAP2 		0x2B /* INT_LOW -- AWAKE -- INACT -- ACT -- FIFO_OVERRUN -- FIFO_WATERMARK -- FIFO_READY -- DATA_READY */
#define ADXL_REG_FILTER_CTL 	0x2C /* Write FFxx xxxx (FF = 00 for +-2g, 01 for =-4g, 1x for +- 8g) for measurement range selection */
#define ADXL_REG_POWER_CTL 		0x2D /* Write xxxx xxMM (MM = 10) to: measurement mode */


/* Prototypes for methods available to be used elsewhere */
void initADXL (void);

void ADXL_setTriggered (bool triggered);
bool ADXL_getTriggered (void);
void ADXL_ackInterrupt (void);

void ADXL_enableSPI (bool enabled);
void ADXL_enableMeasure (bool enabled);

void ADXL_configRange (uint8_t givenRange);
void ADXL_configODR (uint8_t givenODR);
void ADXL_configActivity (uint8_t gThreshold);

void ADXL_readValues (void);


#endif /* _ADXL362_H_ */
