/***************************************************************************//**
 * @file adc.c
 * @brief ADC functionality for reading the (battery) voltage and internal temperature.
 * @version 1.5
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Moved ADC functionality from `other.c` to this file.
 *   @li v1.1: Removed re-initialization dbprint messages.
 *   @li v1.2: Started using custom enum type and cleaned up some unnecessary statements after testing.
 *   @li v1.3: Changed error numbering.
 *   @li v1.4: Added timeout to `while` loop and changed types to `int32_t`.
 *   @li v1.5: Refined timout functionality.
 *
 * ******************************************************************************
 *
 * @section License
 *
 *   Some methods use code obtained from examples from [Silicon Labs' GitHub](https://github.com/SiliconLabs/peripheral_examples).
 *   These sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


#include <stdint.h>        /* (u)intXX_t */
#include <stdbool.h>       /* "bool", "true", "false" */
#include "em_device.h"     /* Include necessary MCU-specific header file */
#include "em_cmu.h"        /* Clock management unit */
#include "em_adc.h"        /* Analog to Digital Converter */

#include "adc.h"           /* Corresponding header file */
#include "debug_dbprint.h" /* Enable or disable printing to UART for debugging */
#include "util.h"          /* Utility functionality */


/* Local definitions */
/** Enable (1) or disable (0) printing the timeout counter value using DBPRINT */
#define DBPRINT_TIMEOUT 0

/** Maximum value for the counter before exiting a `while` loop */
#define TIMEOUT_CONVERSION 50


/* Local variables */
static volatile bool adcConversionComplete = false; /* Volatile because it's modified by an interrupt service routine */
static ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
static ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;


/* Local prototype */
static float32_t convertToCelsius (int32_t adcSample);


/**************************************************************************//**
 * @brief
 *   Method to initialize the ADC to later check the battery voltage or internal temperature.
 *
 * @param[in] peripheral
 *   Select the ADC peripheral to initialize.
 *****************************************************************************/
void initADC (ADC_Measurement_t peripheral)
{
	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* ADC0 is a High Frequency Peripheral */
	CMU_ClockEnable(cmuClock_ADC0, true);

	/* Set a timebase providing at least 1 us.
	 * If the argument is "0" the currently defined HFPER clock setting is used. */
	init.timebase = ADC_TimebaseCalc(0);

	/* Set a prescale value according to the ADC frequency (400 000 Hz) wanted.
	 * If the last argument is "0" the currently defined HFPER clock setting is for the calculation used. */
	init.prescale = ADC_PrescaleCalc(400000, 0);

	/* Initialize ADC peripheral */
	ADC_Init(ADC0, &init);

	/* Setup single conversions */

	/* initSingle.acqTime = adcAcqTime16;
	 * The statement above was found in a SiLabs example but DRAMCO disabled it.
	 * After testing this seemed to have no real effect so it was disabled.
	 * This is probably not necessary since a prescale value other than 0 (default) has been defined. */
	if (peripheral == INTERNAL_TEMPERATURE) initSingle.input = adcSingleInpTemp; /* Internal temperature */
	else if (peripheral == BATTERY_VOLTAGE) initSingle.input = adcSingleInpVDDDiv3; /* Internal VDD/3 */
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Unknown ADC peripheral selected!");
#endif /* DEBUG_DBPRINT */

		error(11);
	}

	ADC_InitSingle(ADC0, &initSingle);

	/* Manually set some calibration values
	 * ADC0->CAL = (0x7C << _ADC_CAL_SINGLEOFFSET_SHIFT) | (0x1F << _ADC_CAL_SINGLEGAIN_SHIFT);
	 * The statement above was found in a SiLabs example but DRAMCO disabled it.
	 * After testing this seemed to throw off the first measurement so it was disabled. */

	/* Enable interrupt on completed conversion */
	ADC_IntEnable(ADC0, ADC_IEN_SINGLE);
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);

	/* Disable used clock */
	CMU_ClockEnable(cmuClock_ADC0, false);

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
	if (peripheral == INTERNAL_TEMPERATURE) dbinfo("ADC0 initialized for internal temperature");
	else if (peripheral == BATTERY_VOLTAGE) dbinfo("ADC0 initialized for VBAT");
#endif /* DEBUG_DBPRINT */

}


/**************************************************************************//**
 * @brief
 *   Method to read the battery voltage or internal temperature.
 *
 * @details
 *   This method re-initializes the ADC settings if necessary.
 *
 * @param[in] peripheral
 *   Select the ADC peripheral to read from.
 *
 * @return
 *   The measured battery voltage or internal temperature.
 *****************************************************************************/
int32_t readADC (ADC_Measurement_t peripheral)
{
	uint16_t counter = 0; /* Timeout counter */
	int32_t value = 0; /* Value to eventually return */

	/* Enable necessary clock */
	CMU_ClockEnable(cmuClock_ADC0, true);

	/* Change ADC settings if necessary */
	if (peripheral == INTERNAL_TEMPERATURE)
	{
		if (initSingle.input != adcSingleInpTemp)
		{
			initSingle.input = adcSingleInpTemp; /* Internal temperature */
			ADC_InitSingle(ADC0, &initSingle);
		}
	}
	else if (peripheral == BATTERY_VOLTAGE)
	{
		if (initSingle.input != adcSingleInpVDDDiv3)
		{
			initSingle.input = adcSingleInpVDDDiv3; /* Internal VDD/3 */
			ADC_InitSingle(ADC0, &initSingle);
		}
	}
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Unknown ADC peripheral selected!");
#endif /* DEBUG_DBPRINT */

		error(12);

		return (0);
	}

	/* Set variable false just in case */
	adcConversionComplete = false;

	/* Start single ADC conversion */
	ADC_Start(ADC0, adcStartSingle);

	/* Wait until the conversion is completed */
	while ((counter < TIMEOUT_CONVERSION) && !adcConversionComplete) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_CONVERSION)
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Waiting time for ADC conversion reached!");
#endif /* DEBUG_DBPRINT */

		error(13);

		return (0);
	}
#if DBPRINT_TIMEOUT == 1 /* DBPRINT_TIMEOUT */
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbwarnInt("ADC conversion (", counter, ")");
#endif /* DEBUG_DBPRINT */

	}
#endif /* DBPRINT_TIMEOUT */

	/* Get the ADC value */
	value = ADC_DataSingleGet(ADC0);

	/* Disable used clock */
	CMU_ClockEnable(cmuClock_ADC0, false);

	/* Calculate final value according to parameter */
	if (peripheral == INTERNAL_TEMPERATURE)
	{
		float32_t ft = convertToCelsius(value)*1000;
		value = (int32_t) ft;
	}
	else if (peripheral == BATTERY_VOLTAGE)
	{
		float32_t fv = value * 3.75 / 4.096;
		value = (int32_t) fv;
	}

	return (value);
}


 /**************************************************************************//**
  * @brief
  *   Method to convert an ADC value to a temperature value.
  *
  * @note
  *   This is a static method because it's only internally used in this file
  *   and called by other methods if necessary.
  *
  * @param[in] adcSample
  *   The ADC sample to convert to a temperature value.
  *
  * @return
  *   The converted temperature value.
  *****************************************************************************/
static float32_t convertToCelsius (int32_t adcSample)
{
	float32_t temp;

	/* Factory calibration temperature from device information page. */
	int32_t cal_temp_0 = ((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK)
						>> _DEVINFO_CAL_TEMP_SHIFT);

	/* Factory calibration value from device information page. */
	int32_t cal_value_0 = ((DEVINFO->ADC0CAL2 & _DEVINFO_ADC0CAL2_TEMP1V25_MASK)
						 >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

	/* Temperature gradient (from datasheet) */
	float32_t t_grad = -6.27;

	temp = (cal_temp_0 - ((cal_value_0 - adcSample) / t_grad));

	return (temp);
}


/**************************************************************************//**
 * @brief
 *   Interrupt Service Routine for ADC0.
 *****************************************************************************/
void ADC0_IRQHandler (void)
{
	/* Read interrupt flags */
	 uint32_t flags = ADC_IntGet(ADC0);

	/* Clear the ADC0 interrupt flags */
	ADC_IntClear(ADC0, flags);

	/* Indicate that an ADC conversion has been completed */
	adcConversionComplete = true;
}
