/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: rn2483.h
 *      Created: 2018-01-18
 *       Author: Guus Leenders - Modified by Brecht Van Eeckhoudt
 *
 *  Description: Header file for rn2483.c
 */

#ifndef _RN2483_H_
#define _RN2483_H_

#include <stdint.h> /* (u)intXX_t */
#include "lora.h"   /* LoRa functionality */

#define RN2483_COMMANDBUFFER_SIZE 80
#define RN2483_POWER_14DBM 1
#define RN2483_POWER_11DBM 2
#define RN2483_POWER_8DBM 3
#define RN2483_POWER_4DBM 4
#define RN2483_POWER_2DBM 5

typedef enum rn2483_statuses{
	MAC_OK,
	INVALID_PARAM,
	JOIN_ACCEPTED,
	JOIN_DENIED,
	NOT_JOINED,
	NO_FREE_CH,
	SILENT,
	FRAME_COUNTER_ERR_REJOIN_NEEDED,
	BUSY,
	MAC_PAUSED,
	INVALID_DATA_LEN,
	MAC_TX_OK,
	MAC_RX,
	MAC_ERR,
	RADIO_TX_OK,
	RADIO_ERR,
	UNKOWN_ERR,
	RN_TX_TIMEOUT,
	RN_RX_TIMEOUT,
	DATA_RETURNED
}RN2483_Status_t;

void RN2483_Init(void);
RN2483_Status_t RN2483_MacReset(char * receiveBuffer, uint8_t bufferSize);

RN2483_Status_t RN2483_GetHardwareEUI(char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_GetSystemVersion(char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_GetApplicationEUI(char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetDeviceEUI(char * eui, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetApplicationEUI(char * eui, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetApplicationKey(char * key, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetApplicationSessionKey(char * key, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetNetworkSessionKey(char * address, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetDeviceAddress(char * address, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetOutputPower(uint8_t pwr, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_DisableAdaptiveDataRate(char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_DisableAutomaticReplies(char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetDataRate(uint8_t dr, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_JoinOTAA(char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_JoinABP(char * receiveBuffer, uint8_t bufferSize);

RN2483_Status_t RN2483_Setup(LoRaSettings_t settings, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetupOTAA(LoRaSettings_t settings, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_SetupABP(LoRaSettings_t settings, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_TransmitUnconfirmed(uint8_t * data, uint8_t payloadSize, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_TransmitConfirmed(uint8_t * data, uint8_t payloadSize, char * receiveBuffer, uint8_t bufferSize);

RN2483_Status_t RN2483_SaveMac(char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_ResumeMac(char * receiveBuffer, uint8_t bufferSize);

RN2483_Status_t RN2483_Sleep(uint32_t sleepTime, volatile bool * wakeUp, char * receiveBuffer, uint8_t bufferSize);
RN2483_Status_t RN2483_Wake(char * receiveBuffer, uint8_t bufferSize);

#endif /* _RN2483_H_ */
