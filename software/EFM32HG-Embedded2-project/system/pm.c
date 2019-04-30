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
 *         File: pm.c
 *      Created: 2018-03-21
 *       Author: Geoffrey Ottoy
 *
 *  Description: Power management functions. Allows you to enable/disable
 *  	certain subsystems of the LoRa extension board.
 */

#include <em_gpio.h>

#include "pm.h"          /* Corresponding header file */
#include "delay.h"       /* Delay functionality */
#include "pin_mapping.h" /* PORT and PIN definitions */

void PM_Init(void){
	//GPIO_PinModeSet(PM_SENS_GECKO_PORT, PM_SENS_GECKO_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(PM_SENS_EXT_PORT, PM_SENS_EXT_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(PM_RN2483_PORT, PM_RN2483_PIN, gpioModePushPull, 0);
}

void PM_Enable(PM_SubSystem_t pmss){
	switch(pmss){
		case PM_SENS_GECKO: {
			//GPIO_PinOutSet(PM_SENS_GECKO_PORT, PM_SENS_GECKO_PIN);
			//DelayMs(40); // give sensor time to power up
		} break;
		case PM_SENS_EXT: {
			GPIO_PinOutSet(PM_SENS_EXT_PORT, PM_SENS_EXT_PIN);
			delay(40); // give sensor time to power up
		} break;
		case PM_RN2483: {
			GPIO_PinOutSet(PM_RN2483_PORT, PM_RN2483_PIN);
		} break;
		case PM_ALL: {
			//GPIO_PinOutSet(PM_SENS_GECKO_PORT, PM_SENS_GECKO_PIN);
			GPIO_PinOutSet(PM_SENS_EXT_PORT, PM_SENS_EXT_PIN);
			delay(40); // give sensor time to power up
			GPIO_PinOutSet(PM_RN2483_PORT, PM_RN2483_PIN);
		} break;
		default: {
			return;
		}
	}
}

void PM_Disable(PM_SubSystem_t pmss){
	switch(pmss){
		case PM_SENS_GECKO: {
			//GPIO_PinOutClear(PM_SENS_GECKO_PORT, PM_SENS_GECKO_PIN);
		} break;
		case PM_SENS_EXT: {
			GPIO_PinOutClear(PM_SENS_EXT_PORT, PM_SENS_EXT_PIN);
		} break;
		case PM_RN2483: {
			GPIO_PinOutClear(PM_RN2483_PORT, PM_RN2483_PIN);
		} break;
		case PM_ALL: {
			//GPIO_PinOutClear(PM_SENS_GECKO_PORT, PM_SENS_GECKO_PIN);
			GPIO_PinOutClear(PM_SENS_EXT_PORT, PM_SENS_EXT_PIN);
			GPIO_PinOutClear(PM_RN2483_PORT, PM_RN2483_PIN);
		} break;
		default: {
		} break;
	}
}
