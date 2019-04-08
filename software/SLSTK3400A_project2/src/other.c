/***************************************************************************//**
 * @file other.c
 * @brief Cable checking and battery voltage functionality.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Move `checkCable` from main.c to this file and add battery voltage measurement logic.
 *
 *   @todo @li Fix cable-checking method.
 *         @li Use VCOMP?
 *
 ******************************************************************************/


#include <stdint.h>    /* (u)intXX_t */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_cmu.h"    /* Clock management unit */
#include "em_emu.h"    /* Energy Management Unit */
#include "em_gpio.h"   /* General Purpose IO */
#include "em_adc.h"    /* Analog to Digital Converter */

#include "../inc/other.h"       /* Corresponding header file */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */
#include "../inc/debugging.h"   /* Enable or disable printing to UART for debugging */
#include "../inc/delay.h"     	/* Delay functionality */


/* Local variable */
static volatile bool adcConversionComplete = false; /* Volatile because it's modified by an interrupt service routine */


/**************************************************************************//**
 * @brief
 *   Method to initialize the ADC to check the battery voltage.
 *****************************************************************************/
void initVBAT (void)
{
	ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
	ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;

	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* ADC0 is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_ADC0, true);

	/* Initialize ADC peripheral */
	ADC_Init(ADC0, &init);

	/* Setup single conversions for internal VDD/3 */
	initSingle.acqTime = adcAcqTime16;
	initSingle.input   = adcSingleInpVDDDiv3;
	ADC_InitSingle(ADC0, &initSingle);

	/* Manually set some calibration values */
	ADC0->CAL = (0x7C << _ADC_CAL_SINGLEOFFSET_SHIFT) | (0x1F << _ADC_CAL_SINGLEGAIN_SHIFT);

	/* Enable interrupt on completed conversion */
	ADC_IntEnable(ADC0, ADC_IEN_SINGLE);
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);

	/* Disable used clock */
	CMU_ClockEnable(cmuClock_ADC0, false);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("ADC0 initialized");
#endif /* DEBUGGING */

}


/**************************************************************************//**
 * @brief
 *   Method to read the battery voltage.
 *
 * @details
 *   This method starts the ADC and reads the result.
 *
 * @return
 *   The measured battery voltage.
 *****************************************************************************/
uint32_t readVBAT (void)
{
	uint32_t voltage;

	/* Enable necessary clock */
	CMU_ClockEnable(cmuClock_ADC0, true);

	/* Set variable false just in case */
	adcConversionComplete = false;

	/* Start single ADC conversion */
	ADC_Start(ADC0, adcStartSingle);

	/* Wait in EM1 until the conversion is completed */
	while (!adcConversionComplete) EMU_EnterEM1();

	/* Get the ADC value */
	voltage = ADC_DataSingleGet(ADC0);

	/* Disable used clock */
	CMU_ClockEnable(cmuClock_ADC0, false);

	return (voltage);
}


/**************************************************************************//**
 * @brief
 *   Method to check if the wire is broken.
 *
 * @details
 *   This method sets the mode of the pins, checks the connection
 *   between them and also disables them at the end.
 *
 * @return
 *   @li true - The connection is still okay.
 *   @li false - The connection is broken!
 *****************************************************************************/
bool checkCable (void)
{
	/* TODO: Fix this method */

	/* Value to eventually return */
	bool check = false;

	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Change mode of first pin */
	GPIO_PinModeSet(BREAK1_PORT, BREAK1_PIN, gpioModeInput, 1); /* TODO: "1" = filter enabled? */

	/* Change mode of second pin and also set it high with the last argument */
	GPIO_PinModeSet(BREAK2_PORT, BREAK2_PIN, gpioModePushPull, 1);

	delay(50);

	/* Check the connection */
	if (!GPIO_PinInGet(BREAK1_PORT,BREAK1_PIN)) check = true;

	/* Disable the pins */
	GPIO_PinModeSet(BREAK1_PORT, BREAK1_PIN, gpioModeDisabled, 0);
	GPIO_PinModeSet(BREAK2_PORT, BREAK2_PIN, gpioModeDisabled, 0);

#ifdef DEBUGGING /* DEBUGGING */
	if (check) dbinfo("Cable still intact");
	else dbcrit("Cable broken!");
#endif /* DEBUGGING */

	return (check);
}


/**************************************************************************//**
 * @brief
 *   Interrupt Service Routine for ADC0.
 *****************************************************************************/
void ADC0_IRQHandler(void)
{
	/* Read interrupt flags */
	 uint32_t flags = ADC_IntGet(ADC0);

	/* Clear the ADC0 interrupt flags */
	ADC_IntClear(ADC0, flags);

	/* Indicate that an ADC conversion has been completed */
	adcConversionComplete = true;
}
