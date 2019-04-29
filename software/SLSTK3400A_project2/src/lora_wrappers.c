/***************************************************************************//**
 * @file lora_wrappers.c
 * @brief LoRa wrapper methods
 * @version 1.2
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
 *   @li v1.2: Updated code to use new functionality to add data to the LPP packet.
 *
 *   @todo
 *     - Save LoRaWAN settings before calling `disableLoRaWAN`?
 *     - Fix `sleepLoRaWAN` and `wakeLoRaWAN` methods.
 *     - Remove unnecessary defines/variables.
 *
 ******************************************************************************/


#include <stdlib.h>         /* "round" and memory functionality */
#include <stdbool.h>        /* "bool", "true", "false" */
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
 *   The measurements get added to the LPP packet following the *custom message
 *   convention* to save bytes to send.
 *
 * @param[in] data
 *   The struct which contains the measurements to send using LoRaWAN.
 *****************************************************************************/
void sendMeasurements (MeasurementData_t data)
{
	/* Initialize LPP-formatted payload
	 * For 6 measurements we need a max amount of 43 bytes (see `LPP_AddMeasurements` method documentation for the calculation) */
	if (!LPP_InitBuffer(&appData, 43)) error(21);

	/* Add measurements to the LPP packet using the custom convention to save bytes send */
	if (!LPP_AddMeasurements(&appData, data)) error(22);

	/* Send custom LPP-like-formatted payload */
	if (LoRa_SendLppBuffer(appData, LORA_UNCONFIMED) != SUCCESS) error(23);
}


/**************************************************************************//**
 * @brief
 *   Send a packet to the cloud using LoRaWAN to indicate that a storm has been detected.
 *
 * @details
 *   The value gets added to the LPP packet following the *custom message convention*.
 *
 * @param[in] stormDetected
 *   @li `true` - A storm has been detected!
 *   @li `false` - No storm is detected.
 *****************************************************************************/
void sendStormDetected (bool stormDetected)
{
	/* Initialize LPP-formatted payload - We need 4 bytes */
	if (!LPP_InitBuffer(&appData, 4)) error(24);

	/* Add value to the LPP packet using the custom convention */
	if (!LPP_AddStormDetected(&appData, stormDetected)) error(25);

	/* Send custom LPP-like-formatted payload */
	if (LoRa_SendLppBuffer(appData, LORA_UNCONFIMED) != SUCCESS) error(26);
}


/**************************************************************************//**
 * @brief
 *   Send a packet to the cloud using LoRaWAN to indicate that the cable is broken.
 *
 * @details
 *   The value gets added to the LPP packet following the *custom message convention*.
 *
 * @param[in] cableBroken
 *   @li `true` - The cable is intact.
 *   @li `false` - The cable is broken!
 *****************************************************************************/
void sendCableBroken (bool cableBroken)
{
	/* Initialize LPP-formatted payload - We need 4 bytes */
	if (!LPP_InitBuffer(&appData, 4)) error(27);

	/* Add value to the LPP packet using the custom convention */
	if (!LPP_AddCableBroken(&appData, cableBroken)) error(28);

	/* Send custom LPP-like-formatted payload */
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
	/* Initialize LPP-formatted payload - We need 4 bytes */
	if (!LPP_InitBuffer(&appData, 4)) error(30);

	/* Add value to the LPP packet using the custom convention */
	if (!LPP_AddStatus(&appData, status)) error(31);

	/* Send custom LPP-like-formatted payload */
	if (LoRa_SendLppBuffer(appData, LORA_UNCONFIMED) != SUCCESS) error(32);
}
