/***************************************************************************//**
 * @file lora_wrappers.h
 * @brief LoRa wrapper methods
 * @version 1.6
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _LORA_WAPPERS_H_
#define _LORA_WAPPERS_H_


/* Include necessary for this header file */
#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "datatypes.h" /* Definitions of the custom data-types */


/* Public prototypes */
void initLoRaWAN (void);
void disableLoRaWAN (void);

void sleepLoRaWAN (uint32_t sSleep);
void wakeLoRaWAN (void);

void sendMeasurements (MeasurementData_t data);
void sendStormDetected (bool stormDetected);
void sendCableBroken (bool cableBroken);
void sendStatus (uint8_t status);

void sendTest (MeasurementData_t data);


#endif /* _LORA_WAPPERS_H_ */
