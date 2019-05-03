/***************************************************************************//**
 * @file ADXL362.c
 * @brief All code for the ADXL362 accelerometer.
 * @version 2.5
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Started with the code from https://github.com/Fescron/Project-LabEmbeddedDesign1/tree/master/code/SLSTK3400A_ADXL362
 *             Changed file name from accel.c to ADXL362.c.
 *   @li v1.1: Changed PinModeSet `out` value to 0 in initADXL_VCC.
 *   @li v1.2: Changed last argument in GPIO_PinModeSet in method initADXL_VCC to
 *             change the pin mode and enable the pin in one statement.
 *   @li v1.3: Changed some methods and global variables to be static (~hidden).
 *   @li v1.4: Changed delay method and cleaned up includes.
 *   @li v1.5: Added get/set method for the static variable `ADXL_triggered`.
 *   @li v1.6: Changed a lot of things...
 *   @li v1.7: Updated documentation and chanced `USART0` to `ADXL_SPI`.
 *   @li v1.8: Updated code with new DEFINE checks.
 *   @li v1.9: Started using custom enum for range & ODR configuration methods.
 *   @li v2.0: Added testing method to go through all the settings, moved the register definitions.
 *   @li v2.1: Disabled SPI pins on *hard* reset, cleaned up some code.
 *   @li v2.2: Added functionality to check the number of interrupts.
 *   @li v2.3: Updated documentation.
 *   @li v2.4: Changed error numbering.
 *   @li v2.5: Updated ODR enum and changed masking logic for register settings.
 *
 *   @todo
 *     - Check initialization (settings)
 *         - Check configurations by reading the registers again?
 *         - Check difference absolute/referenced mode
 *     - Enable wake-up mode?
 *         - `writeADXL(ADXL_REG_POWER_CTL, 0b00001000); // 5th bit`
 *     - Enable loop mode (interrupts don't need to be acknowledged by host)
 *         - `writeADXL(ADXL_REG_ACT_INACT_CTL, 0b00110000);`
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
#include "em_cmu.h"        /* Clock Management Unit */
#include "em_gpio.h"       /* General Purpose IO (GPIO) peripheral API */
#include "em_usart.h"      /* Universal synchr./asynchr. receiver/transmitter (USART/UART) Peripheral API */

#include "ADXL362.h"       /* Corresponding header file */
#include "pin_mapping.h"   /* PORT and PIN definitions */
#include "debug_dbprint.h" /* Enable or disable printing to UART */
#include "delay.h"         /* Delay functionality */
#include "util.h"          /* Utility functionality */


/* Local definitions - ADXL362 register definitions */
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


/* Local variables */
static volatile bool ADXL_triggered = false; /* Volatile because it's modified by an interrupt service routine */
static volatile uint16_t ADXL_triggercounter = 0; /* Volatile because it's modified by an interrupt service routine */
static int8_t XYZDATA[3] = { 0x00, 0x00, 0x00 };
static ADXL_Range_t range;
static bool ADXL_VDD_initialized = false;


/* Local prototypes */
static void powerADXL (bool enabled);
static void initADXL_SPI (void);
static void softResetADXL (void);
static void resetHandlerADXL (void);
static uint8_t readADXL (uint8_t address);
static void writeADXL (uint8_t address, uint8_t data);
static void readADXL_XYZDATA (void);
static bool checkID_ADXL (void);
static int32_t convertGRangeToGValue (int8_t sensorValue);


/**************************************************************************//**
 * @brief
 *   Initialize the accelerometer.
 *
 * @details
 *   This method calls all the other internal necessary functions.
 *   Clock enable functionality is gathered here instead of in
 *   *lower* (static) functions.
 *****************************************************************************/
void initADXL (void)
{
	/* Enable necessary clocks (just in case) */
	CMU_ClockEnable(cmuClock_HFPER, true); /* GPIO and USART0/1 are High Frequency Peripherals */
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Initialize and power VDD pin */
	powerADXL(true);

	/* Power-up delay of 40 ms */
	delay(40);

	/* Enable necessary clock (just in case) */
	if (ADXL_SPI == USART0) CMU_ClockEnable(cmuClock_USART0, true);
	else if (ADXL_SPI == USART1) CMU_ClockEnable(cmuClock_USART1, true);
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Wrong peripheral selected!");
#endif /* DEBUG_DBPRINT */

		error(21);

	}

	/* Initialize USART0/1 as SPI slave (also initialize CS pin) */
	initADXL_SPI();

	/* Soft reset ADXL handler */
	resetHandlerADXL();
}


/**************************************************************************//**
 * @brief
 *   Getter for the `ADXL_triggercounter` static variable.
 *
 * @return
 *   The value of `ADXL_triggercounter`.
 *****************************************************************************/
uint16_t ADXL_getCounter (void)
{
	return (ADXL_triggercounter);
}


/**************************************************************************//**
 * @brief
 *   Method to set the `ADXL_triggercounter` static variable back to zero.
 *****************************************************************************/
void ADXL_clearCounter (void)
{
	ADXL_triggercounter = 0;
}


/**************************************************************************//**
 * @brief
 *   Setter for the `ADXL_triggered` static variable.
 *
 * @param[in] triggered
 *    @li `true` - Set `ADXL_triggered` to `true`.
 *    @li `false` - Set `ADXL_triggered` to `false`.
 *****************************************************************************/
void ADXL_setTriggered (bool triggered)
{
	ADXL_triggered = triggered;
	ADXL_triggercounter++;
}


/**************************************************************************//**
 * @brief
 *   Getter for the `ADXL_triggered` static variable.
 *
 * @return
 *   The value of `ADXL_triggered`.
 *****************************************************************************/
bool ADXL_getTriggered (void)
{
	return (ADXL_triggered);
}


/**************************************************************************//**
 * @brief
 *   Acknowledge the interrupt from the accelerometer.
 *
 * @details
 *   Read a certain register (necessary if the accelerometer is not in
 *   linked-loop mode) and clear the static variable.
 *****************************************************************************/
void ADXL_ackInterrupt (void)
{
	readADXL(ADXL_REG_STATUS);
	ADXL_triggered = false;
}


/**************************************************************************//**
 * @brief
 *   Enable or disable the SPI pins and USART0/1 clock and peripheral to the accelerometer.
 *
 * @param[in] enabled
 *   @li `true` - Enable the SPI pins and USART0/1 clock and peripheral to the accelerometer.
 *   @li `false` - Disable the SPI pins and USART0/1 clock and peripheral to the accelerometer.
 *****************************************************************************/
void ADXL_enableSPI (bool enabled)
{
	if (enabled)
	{
		/* Enable USART clock and peripheral */
		if (ADXL_SPI == USART0) CMU_ClockEnable(cmuClock_USART0, true);
		else if (ADXL_SPI == USART1) CMU_ClockEnable(cmuClock_USART1, true);
		else
		{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
			dbcrit("Wrong peripheral selected!");
#endif /* DEBUG_DBPRINT */

			error(22);

		}
		USART_Enable(ADXL_SPI, usartEnable);

		/* In the case of gpioModePushPull", the last argument directly sets the pin state */
		GPIO_PinModeSet(ADXL_CLK_PORT, ADXL_CLK_PIN, gpioModePushPull, 0);   /* US0_CLK is push pull */
		GPIO_PinModeSet(ADXL_NCS_PORT, ADXL_NCS_PIN, gpioModePushPull, 1);   /* US0_CS is push pull */
		GPIO_PinModeSet(ADXL_MOSI_PORT, ADXL_MOSI_PIN, gpioModePushPull, 1); /* US0_TX (MOSI) is push pull */
		GPIO_PinModeSet(ADXL_MISO_PORT, ADXL_MISO_PIN, gpioModeInput, 1);    /* US0_RX (MISO) is input */
	}
	else
	{
		/* Disable USART clock and peripheral */
		if (ADXL_SPI == USART0) CMU_ClockEnable(cmuClock_USART0, false);
		else if (ADXL_SPI == USART1) CMU_ClockEnable(cmuClock_USART1, false);
		else
		{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
			dbcrit("Wrong peripheral selected!");
#endif /* DEBUG_DBPRINT */

			error(23);

		}
		USART_Enable(ADXL_SPI, usartDisable);

		/* gpioModeDisabled: Pull-up if DOUT is set. */
		GPIO_PinModeSet(ADXL_CLK_PORT, ADXL_CLK_PIN, gpioModeDisabled, 0);
		GPIO_PinModeSet(ADXL_NCS_PORT, ADXL_NCS_PIN, gpioModeDisabled, 1);
		GPIO_PinModeSet(ADXL_MOSI_PORT, ADXL_MOSI_PIN, gpioModeDisabled, 1);
		GPIO_PinModeSet(ADXL_MISO_PORT, ADXL_MISO_PIN, gpioModeDisabled, 1);
	}
}


/**************************************************************************//**
 * @brief
 *   Enable or disable measurement mode.
 *
 * @param[in] enabled
 *   @li `true` - Enable measurement mode.
 *   @li `false` - Disable measurement mode (standby).
 *****************************************************************************/
void ADXL_enableMeasure (bool enabled)
{
	if (enabled)
	{
		/* Get value in register */
		uint8_t reg = readADXL(ADXL_REG_POWER_CTL);

		/* AND with mask to keep the bits we don't want to change */
		reg &= 0b11111100;

		/* Enable measurements (OR with new setting bits) */
		writeADXL(ADXL_REG_POWER_CTL, reg | 0b00000010); /* Last 2 bits are measurement mode */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbinfo("ADXL362: Measurement enabled");
#endif /* DEBUG_DBPRINT */

	}
	else
	{
		/* Get value in register */
		uint8_t reg = readADXL(ADXL_REG_POWER_CTL);

		/* AND with mask to keep the bits we don't want to change */
		reg &= 0b11111100;

		/* Disable measurements (OR with new setting bits) */
		writeADXL(ADXL_REG_POWER_CTL, reg | 0b00000000); /* Last 2 bits are measurement mode */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbinfo("ADXL362: Measurement disabled (standby)");
#endif /* DEBUG_DBPRINT */

	}
}


/**************************************************************************//**
 * @brief
 *   Configure the measurement range and store the selected one in
 *   a global variable for later (internal) use.
 *
 * @details
 *   When a range of, for example "2g" is selected, the real range is "+-2g".
 *
 * @param[in] givenRange
 *   The selected range.
 *****************************************************************************/
void ADXL_configRange (ADXL_Range_t givenRange)
{
	/* Get value in register */
	uint8_t reg = readADXL(ADXL_REG_FILTER_CTL);

	/* AND with mask to keep the bits we don't want to change */
	reg &= 0b00111111;

	/* Set measurement range (OR with new setting bits, first two bits) */
	if (givenRange == ADXL_RANGE_2G)
	{
		writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000000));
		range = ADXL_RANGE_2G;
	}
	else if (givenRange == ADXL_RANGE_4G)
	{
		writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b01000000));
		range = ADXL_RANGE_4G;
	}
	else if (givenRange == ADXL_RANGE_8G)
	{
		writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b10000000));
		range = ADXL_RANGE_8G;
	}
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Non-existing range selected!");
#endif /* DEBUG_DBPRINT */

		error(24);
	}

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
	if (range == ADXL_RANGE_2G) dbinfo("ADXL362: Measurement mode +- 2g selected");
	else if (range == ADXL_RANGE_4G) dbinfo("ADXL362: Measurement mode +- 4g selected");
	else if (range == ADXL_RANGE_8G) dbinfo("ADXL362: Measurement mode +- 8g selected");
#endif /* DEBUG_DBPRINT */

}


/**************************************************************************//**
 * @brief
 *   Configure the Output Data Rate (ODR).
 *
 * @param[in] givenODR
 *   The selected ODR.
 *****************************************************************************/
void ADXL_configODR (ADXL_ODR_t givenODR)
{
	/* Get value in register */
	uint8_t reg = readADXL(ADXL_REG_FILTER_CTL);

	/* AND with mask to keep the bits we don't want to change */
	reg &= 0b11111000;

	/* Set ODR (OR with new setting bits, last three bits) */
	if (givenODR == ADXL_ODR_12_5_HZ) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000000));
	else if (givenODR == ADXL_ODR_25_HZ) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000001));
	else if (givenODR == ADXL_ODR_50_HZ) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000010));
	else if (givenODR == ADXL_ODR_100_HZ) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000011));
	else if (givenODR == ADXL_ODR_200_HZ) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000100));
	else if (givenODR == ADXL_ODR_400_HZ) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000101));
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Non-existing ODR selected!");
#endif /* DEBUG_DBPRINT */

		error(25);
	}

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
	if (givenODR == ADXL_ODR_12_5_HZ) dbinfo("ADXL362: ODR set at 12.5 Hz");
	else if (givenODR == ADXL_ODR_25_HZ) dbinfo("ADXL362: ODR set at 25 Hz");
	else if (givenODR == ADXL_ODR_50_HZ) dbinfo("ADXL362: ODR set at 50 Hz");
	else if (givenODR == ADXL_ODR_100_HZ) dbinfo("ADXL362: ODR set at 100 Hz");
	else if (givenODR == ADXL_ODR_200_HZ) dbinfo("ADXL362: ODR set at 200 Hz");
	else if (givenODR == ADXL_ODR_400_HZ) dbinfo("ADXL362: ODR set at 400 Hz");
#endif /* DEBUG_DBPRINT */

}


/**************************************************************************//**
 * @brief
 *   Configure the accelerometer to work in (referenced) activity threshold mode.
 *
 * @details
 *   Route activity detector to INT1 pin using INTMAP1, isolate bits
 *   and write settings to both threshold registers.
 *
 *   **Referenced** means that during the initialization a *reference acceleration* gets
 *   measured (like for example `1 g` on a certain axis) and stored internally. This value
 *   always gets internally subtracted from a measured acceleration value to calculate
 *   the final value and check if it exceeds the set threshold:
 *     - `ABS(acceleration - reference) > threshold`
 *
 * @param[in] gThreshold
 *   Threshold [g].
 *****************************************************************************/
void ADXL_configActivity (uint8_t gThreshold)
{
	/* Map activity detector to INT1 pin  */
	writeADXL(ADXL_REG_INTMAP1, 0b00010000); /* Bit 4 selects activity detector */

	/* Enable referenced activity threshold mode (last two bits) */
	writeADXL(ADXL_REG_ACT_INACT_CTL, 0b00000011);

	/* Convert g value to "codes"
	 *   THRESH_ACT [codes] = Threshold Value [g] × Scale Factor [LSB per g] */
	uint8_t threshold;

	if (range == ADXL_RANGE_2G) threshold = gThreshold * 1000;
	else if (range == ADXL_RANGE_4G) threshold = gThreshold * 500;
	else if (range == ADXL_RANGE_8G) threshold = gThreshold * 250;
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Range wrong, can't set gThreshold!");
#endif /* DEBUG_DBPRINT */

		error(26);
	}

	/* Isolate bits using masks and shifting */
	uint8_t low  = (threshold & 0b0011111111);
	uint8_t high = (threshold & 0b1100000000) >> 8;

	/* Set threshold register values (total: 11bit unsigned) */
	writeADXL(ADXL_REG_THRESH_ACT_L, low);  /* 7:0 bits used */
	writeADXL(ADXL_REG_THRESH_ACT_H, high); /* 2:0 bits used */

	/* Enable loop mode (interrupts don't need to be acknowledged by host) */
	// writeADXL(ADXL_REG_ACT_INACT_CTL, 0b00110000); TODO: not working...

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
	dbinfoInt("ADXL362: Activity configured: ", gThreshold, "g");
#endif /* DEBUG_DBPRINT */

}


/**************************************************************************//**
 * @brief
 *   Read and display "g" values forever with a 100ms interval.
 *****************************************************************************/
void ADXL_readValues (void)
{
	uint32_t counter = 0;

	/* Enable measurement mode */
	ADXL_enableMeasure(true);

	/* Infinite loop */
	while (1)
	{
		led(true); /* Enable LED */

		readADXL_XYZDATA(); /* Read XYZ sensor data */

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		/* Print XYZ sensor data */
		dbprint("\r[");
		dbprintInt(counter);
		dbprint("] X: ");
		dbprintInt(convertGRangeToGValue(XYZDATA[0]));
		dbprint(" mg | Y: ");
		dbprintInt(convertGRangeToGValue(XYZDATA[1]));
		dbprint(" mg | Z: ");
		dbprintInt(convertGRangeToGValue(XYZDATA[2]));
		dbprint(" mg       "); /* Extra spacing is to overwrite other data if it's remaining (see \r) */
#endif /* DEBUG_DBPRINT */

		led(false); /* Disable LED */

		counter++;

		delay(100);
	}
}


/**************************************************************************//**
 * @brief
 *   This method goes through all of the ODR settings to see the influence
 *   they have on power usage. The measurement range is the default one (+-2g).
 *
 * @details
 *   To get the "correct" currents the delay method puts the MCU to EM2/3 sleep
 *   and the SPI lines are disabled. The order of the test is:
 *     - Wait 5 seconds
 *     - Soft reset the accelerometer
 *     - One second in "standby" mode
 *     - Enable "measurement" mode
 *     - One second in ODR 12.5 Hz
 *     - One second in ODR 25 Hz
 *     - One second in ODR 50 Hz
 *     - One second in ODR 100 Hz
 *     - One second in ODR 200 Hz
 *     - One second in ODR 400 Hz
 *     - Soft reset the accelerometer
 *****************************************************************************/
void testADXL (void)
{
	delay(5000);

	/* Soft reset ADXL */
	softResetADXL();

	/* Standby */
	ADXL_enableSPI(false); /* Disable SPI functionality */
	delay(1000);
	ADXL_enableSPI(true);  /* Enable SPI functionality */

	/* ODR 12,5Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010000); /* last 3 bits are ODR */

	/* Enable measurements */
	writeADXL(ADXL_REG_POWER_CTL, 0b00000010); /* last 2 bits are measurement mode */

	ADXL_enableSPI(false); /* Disable SPI functionality */
	delay(1000);
	ADXL_enableSPI(true);  /* Enable SPI functionality */

	/* ODR 25Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010001); /* last 3 bits are ODR */

	ADXL_enableSPI(false); /* Disable SPI functionality */
	delay(1000);
	ADXL_enableSPI(true);  /* Enable SPI functionality */

	/* ODR 50Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010010); /* last 3 bits are ODR */

	ADXL_enableSPI(false); /* Disable SPI functionality */
	delay(1000);
	ADXL_enableSPI(true);  /* Enable SPI functionality */

	/* ODR 100Hz (default) */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010011); /* last 3 bits are ODR */

	ADXL_enableSPI(false); /* Disable SPI functionality */
	delay(1000);
	ADXL_enableSPI(true);  /* Enable SPI functionality */

	/* ODR 200Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010100); /* last 3 bits are ODR */

	ADXL_enableSPI(false); /* Disable SPI functionality */
	delay(1000);
	ADXL_enableSPI(true);  /* Enable SPI functionality */

	/* ODR 400Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010101); /* last 3 bits are ODR */

	ADXL_enableSPI(false); /* Disable SPI functionality */
	delay(1000);
	ADXL_enableSPI(true);  /* Enable SPI functionality */

	/* Soft reset ADXL */
	softResetADXL();
}


/**************************************************************************//**
 * @brief
 *   Initialize USARTx in SPI mode according to the settings required
 *   by the accelerometer.
 *
 * @details
 *   Configure pins, configure USARTx in SPI mode, route
 *   the pins, enable USARTx and set CS high.
 *   Necessary clocks are enabled in a previous method.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *****************************************************************************/
static void initADXL_SPI (void)
{
	/* Configure GPIO */
	/* In the case of gpioModePushPull", the last argument directly sets the pin state */
	GPIO_PinModeSet(ADXL_CLK_PORT, ADXL_CLK_PIN, gpioModePushPull, 0);   /* US0_CLK is push pull */
	GPIO_PinModeSet(ADXL_NCS_PORT, ADXL_NCS_PIN, gpioModePushPull, 1);   /* US0_CS is push pull */
	GPIO_PinModeSet(ADXL_MOSI_PORT, ADXL_MOSI_PIN, gpioModePushPull, 1); /* US0_TX (MOSI) is push pull */
	GPIO_PinModeSet(ADXL_MISO_PORT, ADXL_MISO_PIN, gpioModeInput, 1);    /* US0_RX (MISO) is input */

	/* Start with default config */
	USART_InitSync_TypeDef config = USART_INITSYNC_DEFAULT;

	/* Modify some settings */
	config.enable       = false;           	/* making sure to keep USART disabled until we've set everything up */
	config.refFreq      = 0;			 	/* USART/UART reference clock assumed when configuring baud rate setup. Set to 0 to use the currently configured reference clock. */
	config.baudrate     = 4000000;         	/* CLK freq is 1 MHz (1000000) */
	config.databits     = usartDatabits8;	/* master mode */
	config.master       = true;            	/* master mode */
	config.msbf         = true;            	/* send MSB first */
	config.clockMode    = usartClockMode0; 	/* clock idle low, sample on rising/first edge (Clock polarity/phase mode = CPOL/CPHA) */
	config.prsRxEnable = false;				/* If enabled: Enable USART Rx via PRS. */
	config.autoTx = false; 					/* If enabled: Enable AUTOTX mode. Transmits as long as RX is not full. Generates underflows if TX is empty. */
	config.autoCsEnable = false;            /* CS pin controlled by hardware, not firmware */

	/* Initialize USART0/1 with the configured parameters */
	USART_InitSync(ADXL_SPI, &config);

	/* Set the pin location for USART0/1 (before: USART_ROUTE_LOCATION_LOC0) */
	ADXL_SPI->ROUTE = USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | ADXL_SPI_LOC;

	/* Enable USART0/1 */
	USART_Enable(ADXL_SPI, usartEnable);

	/* Set CS high (active low!) */
	GPIO_PinOutSet(gpioPortE, 13);
}


/**************************************************************************//**
 * @brief
 *   Soft reset accelerometer handler.
 *
 * @details
 *   If the first ID check fails, the MCU is put on hold for one second
 *   and the ID gets checked again.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *****************************************************************************/
static void resetHandlerADXL (void)
{
	uint8_t retries = 0;

	/* Soft reset ADXL */
	softResetADXL();

	/* First try to get the correct ID failed */
	if (!checkID_ADXL())
	{
		retries++;

		delay(1000);

		/* Soft reset */
		softResetADXL();

		/* Second try to get the correct ID failed */
		if (!checkID_ADXL())
		{
			retries++;

			delay(1000);

			/* Soft reset */
			softResetADXL();

			/* Third try to get the correct ID failed
			 * resorting to "hard" reset! */
			if (!checkID_ADXL())
			{
				retries++;

				powerADXL(false);
				ADXL_enableSPI(false); /* Make sure the accelerometer doesn't get power through the SPI pins */

				delay(1000);

				powerADXL(true);
				ADXL_enableSPI(true);

				delay(1000);

				/* Soft reset */
				softResetADXL();

				/* Last try to get the correct ID failed */
				if (!checkID_ADXL())
				{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
					dbcrit("ADXL362 initialization failed");
#endif /* DEBUG_DBPRINT */

					error(20);
				}
			}
		}
	}

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
	if (retries < 2) dbinfoInt("ADXL362 initialized (", retries, " soft reset retries)");
	else dbwarnInt("ADXL362 initialized (had to \"hard reset\", ", retries, " soft reset retries)");
#endif /* DEBUG_DBPRINT */

}


/**************************************************************************//**
 * @brief
 *   Read an SPI byte from the accelerometer (8 bits) using a given address.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] address
 *   The register address to read from.
 *
 * @return
 *   The response (one byte, `uint8_t`).
 *****************************************************************************/
static uint8_t readADXL (uint8_t address)
{
	uint8_t response;

	/* Set CS low (active low!) */
	GPIO_PinOutClear(ADXL_NCS_PORT, ADXL_NCS_PIN);

	/* 3-byte operation according to datasheet */
	USART_SpiTransfer(ADXL_SPI, 0x0B);            /* "read" instruction */
	USART_SpiTransfer(ADXL_SPI, address);         /* Address */
	response = USART_SpiTransfer(ADXL_SPI, 0x00); /* Read response */

	/* Set CS high */
	GPIO_PinOutSet(ADXL_NCS_PORT, ADXL_NCS_PIN);

	return (response);
}


/**************************************************************************//**
 * @brief
 *   Write an SPI byte to the accelerometer (8 bits) using a given address
 *   and specified data.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] address
 *   The register address to write the data to (one byte, `uint8_t`).
 *
 * @param[in] data
 *   The data to write to the address (one byte, `uint8_t`).
 *****************************************************************************/
static void writeADXL (uint8_t address, uint8_t data)
{
	/* Set CS low (active low!) */
	GPIO_PinOutClear(ADXL_NCS_PORT, ADXL_NCS_PIN);

	/* 3-byte operation according to datasheet */
	USART_SpiTransfer(ADXL_SPI, 0x0A);    /* "write" instruction */
	USART_SpiTransfer(ADXL_SPI, address); /* Address */
	USART_SpiTransfer(ADXL_SPI, data);    /* Data */

	/* Set CS high */
	GPIO_PinOutSet(ADXL_NCS_PORT, ADXL_NCS_PIN);
}


/**************************************************************************//**
 * @brief
 *   Read the X-Y-Z data registers in the `XYZDATA[]` field using burst reads.
 *
 * @details
 *   Response data gets put in `XYZDATA` array (global volatile variable).
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *****************************************************************************/
static void readADXL_XYZDATA (void)
{
	/* CS low (active low!) */
	GPIO_PinOutClear(ADXL_NCS_PORT, ADXL_NCS_PIN);

	/* Burst read (address auto-increments) */
	USART_SpiTransfer(ADXL_SPI, 0x0B);				/* "read" instruction */
	USART_SpiTransfer(ADXL_SPI, ADXL_REG_XDATA);    /* Address */
	XYZDATA[0] = USART_SpiTransfer(ADXL_SPI, 0x00);	/* Read response */
	XYZDATA[1] = USART_SpiTransfer(ADXL_SPI, 0x00);	/* Read response */
	XYZDATA[2] = USART_SpiTransfer(ADXL_SPI, 0x00);	/* Read response */

	/* CS high */
	GPIO_PinOutSet(ADXL_NCS_PORT, ADXL_NCS_PIN);
}


/**************************************************************************//**
 * @brief
 *   Enable or disable the power to the accelerometer.
 *
 * @details
 *   This method also initializes the pin-mode if necessary.
 *   Necessary clocks are enabled in a previous method.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] enabled
 *   @li `true` - Enable the GPIO pin connected to the VDD pin of the accelerometer.
 *   @li `false` - Disable the GPIO pin connected to the VDD pin of the accelerometer.
 *****************************************************************************/
static void powerADXL (bool enabled)
{
	/* Initialize VDD pin if not already the case */
	if (!ADXL_VDD_initialized)
	{
		/* In the case of gpioModePushPull", the last argument directly sets the pin state */
		GPIO_PinModeSet(ADXL_VDD_PORT, ADXL_VDD_PIN, gpioModePushPull, enabled);

		ADXL_VDD_initialized = true;
	}
	else
	{
		if (enabled) GPIO_PinOutSet(ADXL_VDD_PORT, ADXL_VDD_PIN); /* Enable VDD pin */
		else GPIO_PinOutClear(ADXL_VDD_PORT, ADXL_VDD_PIN); /* Disable VDD pin */
	}
}


/**************************************************************************//**
 * @brief
 *   Soft reset accelerometer.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *****************************************************************************/
static void softResetADXL (void)
{
	writeADXL(ADXL_REG_SOFT_RESET, 0x52); /* 0x52 = "R" */
}


/**************************************************************************//**
 * @brief Check if the ID is correct.
 *
 * @note
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @return
 *   @li `true` - Correct ID returned.
 *   @li `false` - Incorrect ID returned.
 *****************************************************************************/
static bool checkID_ADXL (void)
{
	return (readADXL(ADXL_REG_DEVID_AD) == 0xAD);
}


/**************************************************************************//**
 * @brief
 *   Convert sensor value in +-g range to mg value.
 *
 * @note
 *   Info found at http://ozzmaker.com/accelerometer-to-g/
 *   This is a static method because it's only internally used in this file
 *   and called by other methods if necessary.
 *
 * @param[in] sensorValue
 *   Value in g-range returned by sensor.
 *
 * @return
 *   The calculated mg value.
 *****************************************************************************/
static int32_t convertGRangeToGValue (int8_t sensorValue)
{
	/* Explanation of the used numbers:
	 *   - 255 = (-) 128 + 127
	 *   - 2 = "+" & "-"
	 *   - 1000 = "m"g */

	if (range == ADXL_RANGE_2G) return ((2*2*1000 / 255) * sensorValue);
	else if (range == ADXL_RANGE_4G) return ((2*4*1000 / 255) * sensorValue);
	else if (range == ADXL_RANGE_8G) return ((2*8*1000 / 255) * sensorValue);
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Range wrong, can't calculate mg value!");
#endif /* DEBUG_DBPRINT */

		error(27);

		return (0);
	}
}
