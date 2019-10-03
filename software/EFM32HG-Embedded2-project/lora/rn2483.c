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
 *         File: rn2483.c
 *      Created: 2018-01-18
 *       Author: Guus Leenders - Modified by Brecht Van Eeckhoudt
 *
 *  Description: Interfacing with the RN2483 LoRa modem.
 *  	Wraps commands in a more readable format. Handles serial
 *  	command-response mechanism.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usart.h>
#include <em_leuart.h>

#include "lora.h"        /* LoRa functionality */
#include "leuart.h"      /* leuart functionality */
#include "rn2483.h"      /* RN2483_xxxx */

#include "delay.h"       /* Delay functionality */
#include "pin_mapping.h" /* PORT and PIN definitions */
#include "util_string.h" /* Utility functionality regarding strings */


char commandBuffer[RN2483_COMMANDBUFFER_SIZE];

static RN2483_Status_t RN2483_ProcessMacCommand(char * receiveBuffer, uint8_t bufferSize, bool secondResponse){
	volatile bool dummy = false;
	if(Leuart_SendCommand(commandBuffer, strlen(commandBuffer), &dummy) == TX_TIMEOUT){
		return (RN_TX_TIMEOUT);
	}

	/* Read first response */
	Leuart_ReadResponse(receiveBuffer, bufferSize);

	/* Analyze response */
	if(StringStartsWith(receiveBuffer, "ok")){
		if(secondResponse){
			/* Wait for second response */
			Leuart_WaitForResponse(receiveBuffer, bufferSize);
			/* Read second response */
			Leuart_ReadResponse(receiveBuffer, bufferSize);
			/* Analyze response */
			if(StringStartsWith(receiveBuffer, "accepted")){
				return (JOIN_ACCEPTED);
			} else if(StringStartsWith(receiveBuffer, "denied")){
				return (JOIN_DENIED);
			} else if(StringStartsWith(receiveBuffer, "mac_tx_ok")){
				return (MAC_TX_OK);
			} else if(StringStartsWith(receiveBuffer, "mac_rx")){
				// TODO: capture data
				return (MAC_RX);
			} else if(StringStartsWith(receiveBuffer, "mac_err")){
				return (MAC_ERR);
			} else if(StringStartsWith(receiveBuffer, "invalid_data_len")){
				return (INVALID_DATA_LEN);
			} else if(StringStartsWith(receiveBuffer, "radio_tx_ok")){
				return (RADIO_TX_OK);
			} else if(StringStartsWith(receiveBuffer, "radio_err")){
				return (RADIO_ERR);
			}
			return (UNKOWN_ERR);
		} else {
			return (MAC_OK);
		}
	}
	else if(StringStartsWith(receiveBuffer, "invalid_param")){
		return (INVALID_PARAM);
	} else if(StringStartsWith(receiveBuffer, "not_joined")){
		return (NOT_JOINED);
	} else if(StringStartsWith(receiveBuffer, "no_free_ch")){
		return (NO_FREE_CH);
	} else if(StringStartsWith(receiveBuffer, "silent")){
		return (SILENT);
	} else if(StringStartsWith(receiveBuffer, "frame_counter_err_rejoin_needed")){
		return (FRAME_COUNTER_ERR_REJOIN_NEEDED);
	} else if(StringStartsWith(receiveBuffer, "busy")){
		return (BUSY);
	} else if(StringStartsWith(receiveBuffer, "mac_paused")){
		return (MAC_PAUSED);
	} else if(StringStartsWith(receiveBuffer, "invalid_data_len")){
		return (INVALID_DATA_LEN);
	}

	return (DATA_RETURNED);
}

static RN2483_Status_t RN2483_ProcessSleepCommand(char * receiveBuffer, uint8_t bufferSize, volatile bool * wakeUp){
	if(Leuart_SendCommand(commandBuffer, strlen(commandBuffer), wakeUp) == TX_TIMEOUT){
		return (RN_TX_TIMEOUT);
	}

	/* Read first response */
	Leuart_ReadResponse(receiveBuffer, bufferSize);

	/* Analyze response */
	if(StringStartsWith(receiveBuffer, "ok")){
		return (MAC_OK);
	} else if(StringStartsWith(receiveBuffer, "invalid_param")){
		return (INVALID_PARAM);
	}

	return (UNKOWN_ERR);
}

void RN2483_Init(void){ /* Setup with autobaud */
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 1);
	delay(100);
	GPIO_PinModeSet(RN2483_RESET_PORT, RN2483_RESET_PIN, gpioModePushPull, 1);
	delay(50);
	GPIO_PinModeSet(RN2483_RESET_PORT, RN2483_RESET_PIN, gpioModePushPull, 0);
	delay(50);
	GPIO_PinModeSet(RN2483_RESET_PORT, RN2483_RESET_PIN, gpioModePushPull, 1);
	delay(350);

	memset(commandBuffer, '\0', RN2483_COMMANDBUFFER_SIZE);

	Leuart_Init();
}

RN2483_Status_t RN2483_MacReset(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac reset 868\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_GetHardwareEUI(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac get deveui\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_GetSystemVersion(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "sys get ver\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_GetApplicationEUI(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "sys get appeui\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetDeviceEUI(char * eui, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set deveui %s\r\n", eui);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetApplicationEUI(char * eui, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set appeui %s\r\n", eui);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetApplicationKey(char * key, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set appkey %s\r\n", key);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetApplicationSessionKey(char * key, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set appskey %s\r\n", key);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetNetworkSessionKey(char * key, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set nwkskey %s\r\n", key);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetDeviceAddress(char * address, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set devaddr %s\r\n", address);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetOutputPower(uint8_t pwr, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set pwridx %i\r\n", pwr);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_DisableAdaptiveDataRate(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set adr off\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_DisableAutomaticReplies(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set ar off\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetDataRate(uint8_t dr, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set dr %i\r\n", dr);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_GetDataRate(int8_t *dr, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac get dr\r\n");
	RN2483_Status_t status = RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false);
	if(status != DATA_RETURNED){
		return (status);
	}

	/* Convert ascii to unsigned */
	*dr = receiveBuffer[0] - '0';
	/* Final data check */
	if(*dr >= 0 && *dr <= 7){
		return (MAC_OK);
	}
	else{
		return (MAC_ERR);
	}
}

RN2483_Status_t RN2483_EnableAdaptiveDataRate(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set adr on\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_SetBatteryLevel(uint8_t battery, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac set bat %i\r\n", battery);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_JoinOTAA(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac join otaa\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, true));
}

RN2483_Status_t RN2483_JoinABP(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac join abp\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, true));
}

static RN2483_Status_t RN2483_SetDefaultOperation(LoRaSettings_t settings, char * receiveBuffer, uint8_t bufferSize){
	RN2483_Status_t status = RN2483_SetOutputPower(RN2483_POWER_14DBM, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_DisableAutomaticReplies(receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_DisableAdaptiveDataRate(receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_SetDataRate(settings.dataRate, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_EnableAdaptiveDataRate(receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	return (RN2483_SetBatteryLevel(254, receiveBuffer, bufferSize));
}

RN2483_Status_t RN2483_Setup(LoRaSettings_t settings, char * receiveBuffer, uint8_t bufferSize){
	if(settings.activationMethod == OTAA){
		return (RN2483_SetupOTAA(settings, receiveBuffer, bufferSize));
	}else{
		return (RN2483_SetupABP(settings, receiveBuffer, bufferSize));
	}
}

RN2483_Status_t RN2483_SetupOTAA(LoRaSettings_t settings, char * receiveBuffer, uint8_t bufferSize){
	RN2483_Status_t status = RN2483_MacReset(receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	if(settings.deviceEUI == NULL){
		status = RN2483_GetHardwareEUI(receiveBuffer, bufferSize);
		if(status != DATA_RETURNED){
			return (status);
		}
		char address[16];
		sprintf(address, "%s", receiveBuffer);
		status = RN2483_SetDeviceEUI(address, receiveBuffer, bufferSize);
		if(status != MAC_OK){
			return (status);
		}
	}else{
		status = RN2483_SetDeviceEUI(settings.deviceEUI, receiveBuffer, bufferSize);
		if(status != MAC_OK){
			return (status);
		}
	}
	status = RN2483_SetApplicationEUI(settings.applicationEUI, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_SetApplicationKey(settings.applicationKey, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_SetDefaultOperation(settings, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	return (RN2483_JoinOTAA(receiveBuffer, bufferSize));
}

RN2483_Status_t RN2483_SetupABP(LoRaSettings_t settings, char * receiveBuffer, uint8_t bufferSize){
	RN2483_Status_t status = RN2483_MacReset(receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_SetDeviceAddress(settings.deviceAddress, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_SetNetworkSessionKey(settings.networkSessionKey, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_SetApplicationSessionKey(settings.applicationSessionKey, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	status = RN2483_SetDefaultOperation(settings, receiveBuffer, bufferSize);
	if(status != MAC_OK){
		return (status);
	}
	return (RN2483_JoinABP(receiveBuffer, bufferSize));
}

RN2483_Status_t RN2483_TransmitUnconfirmed(uint8_t * data, uint8_t payloadSize, char * receiveBuffer, uint8_t bufferSize){
	char * encodedPayload;
	if(!HexToString(data, payloadSize, &encodedPayload )){
		return (MAC_ERR);
	}
	//StringToHexString(data, payloadSize/2, &decodedPayload);
	sprintf(commandBuffer, "mac tx uncnf 1 %s\r\n", encodedPayload); // TODO: this seemed to cause the "dbpointer" problem?
	free(encodedPayload);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, true));
}

RN2483_Status_t RN2483_TransmitConfirmed(uint8_t * data, uint8_t payloadSize, char * receiveBuffer, uint8_t bufferSize){
	char * encodedPayload;
	if(!HexToString(data, payloadSize, &encodedPayload )){
		return (MAC_ERR);
	}
	sprintf(commandBuffer, "mac tx cnf 1 %s\r\n", encodedPayload);
	free(encodedPayload);
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, true));
}

RN2483_Status_t RN2483_SaveMac(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac save\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_ResumeMac(char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "mac resume\r\n");
	return (RN2483_ProcessMacCommand(receiveBuffer, bufferSize, false));
}

RN2483_Status_t RN2483_Sleep(uint32_t sleepTime, volatile bool * wakeUp, char * receiveBuffer, uint8_t bufferSize){
	sprintf(commandBuffer, "sys sleep %lu\r\n", (unsigned long) sleepTime);
	return (RN2483_ProcessSleepCommand(receiveBuffer, bufferSize, wakeUp));
}

RN2483_Status_t RN2483_Wake(char * receiveBuffer, uint8_t bufferSize){
	Leuart_Reinit();

	/* Capture response "ok" from previous sleep command */
	Leuart_WaitForResponse();
	/* Read second response */
	Leuart_ReadResponse(receiveBuffer, bufferSize);
	if(StringStartsWith(receiveBuffer, "ok")){
		return (MAC_OK);
	}

	return (MAC_ERR);
}
