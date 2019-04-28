/***************************************************************************//**
 * @file lora_wrappers.c
 * @brief LoRa wrapper methods
 * @version 1.1
 * @author
 *   Benjamin Van der Smissen@n
 *   Heavily modified by Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Started with original code from Benjamin.
 *   @li v1.1: Modified a lot of code, implemented custom LPP data type methods.
 *
 *   @todo
 *     - Fix `sendMeasurements` method!
 *     - Save LoRaWAN settings before calling `disableLoRaWAN`?
 *     - Fix `sleepLoRaWAN` and `wakeLoRaWAN` methods.
 *     - Remove unnecessary defines/variables.
 *
 ******************************************************************************/


#include <stdbool.h>        /* (u)intXX_t */
#include <stdlib.h>         /* "bool", "true", "false" */
#include "em_gpio.h"        /* General Purpose IO */
#include "em_leuart.h"      /* Low Energy Universal Asynchronous Receiver/Transmitter Peripheral API */

#include "lora.h"           /* LoRaWAN functionality */
#include "lpp.h"            /* Basic Low Power Payload (LPP) functionality */
#include "pm.h"             /* Power management functionality */
#include "my_lora_device.h" /* LoRaWAN settings */

#include "lora_wrappers.h"  /* Corresponding header file */
#include "pin_mapping.h"    /* PORT and PIN definitions */
#include "datatypes.h"      /* Definitions of the custom data-types */
#include "util.h"           /* Utility functionality */


/* Local definitions TODO: remove? */
#define A_SECOND 1000
#define A_MINUTE 60000
#define A_HOUR 3600000


/* Local variable TODO: remove? */
volatile bool wakeUp;


/* Application variables */
static LoRaSettings_t loraSettings = LORA_INIT_MY_DEVICE;
static LoRaStatus_t loraStatus;
static LPP_Buffer_t appData;


/**************************************************************************//**
 * @brief
 *   Initialize LoRaWAN functionality.
 *****************************************************************************/
void initLoRaWAN (void)
{
	memset(&appData, 0, sizeof(appData));

	/* Initialize LoRaWAN communication */
	loraStatus = LoRa_Init(loraSettings);

	if (loraStatus != JOINED) error(20);
}


/**************************************************************************//**
 * @brief
 *   Disable LoRaWAN functionality.
 *****************************************************************************/
void disableLoRaWAN (void)
{
	LEUART_Reset(RN2483_UART);
	PM_Disable(PM_RN2483);
	GPIO_PinOutClear(RN2483_RESET_PORT, RN2483_RESET_PIN);
	GPIO_PinOutClear(RN2483_RX_PORT, RN2483_RX_PIN);
	GPIO_PinOutClear(RN2483_TX_PORT, RN2483_TX_PIN);
}


/**************************************************************************//**
 * @brief
 *   Let the LoRaWAN module sleep for a specified amount of time.
 *
 * @param[in] sSleep
 *   The amount of seconds for the module to go to sleep.
 *****************************************************************************/
void sleepLoRaWAN (uint32_t sSleep)
{
	wakeUp = false;
	LoRa_Sleep((A_SECOND*sSleep), &wakeUp);
}


/**************************************************************************//**
 * @brief
 *   Wake up the LoRaWAN module early after putting it to sleep using `sleepLoRaWAN`.
 *****************************************************************************/
void wakeLoRaWAN (void)
{
	LoRa_WakeUp();
}


/**************************************************************************//**
 * @brief
 *   Send measured battery voltages and internal and external temperatures
 *   to the cloud using LoRaWAN.
 *
 * @details
 *   The code detects if not all six measurement fields are filled in the array
 *   and reacts accordingly using the `index` field.
 *
 * @param[in] data
 *   The struct which contains the measurements to send using LoRaWAN.
 *
 * @param[in] stormDetected
 *   A value to indicate if a storm is detected using the accelerometer.
 *****************************************************************************/
void sendMeasurements (MeasurementData_t data, bool stormDetected)
{
	/* Initialize and fill LPP-formatted payload
	 * 6*VBAT (4) + 6*intTemp (4) + 6*extTemp (4) + 1*stormDetected (3) = 75 bytes */
	if (!LPP_InitBuffer(&appData, 75)) error(21);

	/* Add battery voltage measurements to LPP payload */
	for (uint8_t i = 0; i <= data.index-1; i++)
	{
		int16_t batteryLPP = (int16_t)(round((float)data.voltage[i]/10));
		if (!LPP_AddVBAT(&appData, batteryLPP)) error(22);
	}

	/* Add internal temperature measurements to LPP payload */
	for (uint8_t i = 0; i <= data.index-1; i++)
	{
		/* Convert temperature sensor value (should represent 0.1 °C Signed MSB) */
		int16_t intTempLPP = (int16_t)(round((float)data.intTemp[i]/100));
		if (!LPP_AddIntTemp(&appData, intTempLPP)) error(23);
	}

	/* Add external temperature measurements to LPP payload */
	for (uint8_t i = 0; i <= data.index-1; i++)
	{
		/* Convert temperature sensor value (should represent 0.1 °C Signed MSB) */
		int16_t extTempLPP = (int16_t)(round((float)data.extTemp[i]/100));
		if (!LPP_AddExtTemp(&appData, extTempLPP)) error(24);
	}

	/* Add "storm" value */
	if (!LPP_AddStormDetected(&appData, stormDetected)) error(25);

	/* Send LPP-formatted payload */
	LoRaStatus_t test = LoRa_SendLppBuffer(appData, LORA_UNCONFIMED); /* TODO: fix this, can't send (buffer too large?) */
	if (LoRa_SendLppBuffer(appData, LORA_UNCONFIMED) != SUCCESS) error(26);
}


/**************************************************************************//**
 * @brief
 *   Send a packet to indicate the cable is broken.
 *
 * @param[in] cableBroken
 *   @li `true` - The cable is intact.
 *   @li `false` - The cable is broken!
 *****************************************************************************/
void sendCableBroken (bool cableBroken)
{
	/* Initialize and fill LPP-formatted payload
	 * 1*cableBroken (3) = 3 bytes */
	if (!LPP_InitBuffer(&appData, 3)) error(27);

	/* Add cable break value */
    if (!LPP_AddCableBroken(&appData, cableBroken)) error(28);

	/* Send LPP-formatted payload */
	if (LoRa_SendLppBuffer(appData, LORA_UNCONFIMED) != SUCCESS) error(29);
}


/**************************************************************************//**
 * @brief
 *   Send a packet to indicate a *status*.
 *
 * @param[in] status
 *   The status value to send.
 *****************************************************************************/
void sendStatus (uint8_t status)
{
	/* Initialize and fill LPP-formatted payload
	 * 1*status (3) = 3 bytes */
	if (!LPP_InitBuffer(&appData, 3)) error(30);

	/* Add cable break value */
    if (!LPP_AddStatus(&appData, status)) error(31);

	/* Send LPP-formatted payload */
	if (LoRa_SendLppBuffer(appData, LORA_UNCONFIMED) != SUCCESS) error(32);
}
