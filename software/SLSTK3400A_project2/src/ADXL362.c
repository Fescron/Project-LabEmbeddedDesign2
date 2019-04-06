/***************************************************************************//**
 * @file ADXL362.c
 * @brief All code for the ADXL362 accelerometer.
 * @version 1.4
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   v1.0: Started with the code from https://github.com/Fescron/Project-LabEmbeddedDesign1/tree/master/code/SLSTK3400A_ADXL362
 *         Changed file name from accel.c to ADXL362.c.
 *   v1.1: Changed PinModeSet "out" value to 0 in initADXL_VCC.
 *   v1.2: Changed last argument in GPIO_PinModeSet in method initADXL_VCC to
 *         change the pin mode and enable the pin in one statement.
 *   v1.3: Changed some methods and global variables to be static (~hidden).
 *   v1.4: Changed delay method and cleaned up includes.
 *
 *   TODO: Remove stdint and stdbool includes?
 *         Too much movement breaks interrupt functionality, register not cleared
 *         good but new movement already detected?
 *         Make more methods static.
 *
 ******************************************************************************/


/* Includes necessary for this source file */
//#include <stdint.h>     /* (u)intXX_t */
//#include <stdbool.h>    /* "bool", "true", "false" */
#include "em_cmu.h"     /* Clock Management Unit */
#include "em_gpio.h"    /* General Purpose IO (GPIO) peripheral API */
#include "em_usart.h"   /* Universal synchr./asynchr. receiver/transmitter (USART/UART) Peripheral API */

#include "../inc/ADXL362.h"     /* Corresponding header file */
#include "../inc/delay.h"     	/* Delay functionality */
#include "../inc/util.h"     	/* Utility functions */
#include "../inc/handlers.h" 	/* Interrupt handlers */
#include "../inc/pin_mapping.h" /* PORT and PIN definitions */

#include "../inc/debugging.h" /* Enable or disable printing to UART */


/* Global variable */
volatile int8_t XYZDATA[3] = { 0x00, 0x00, 0x00 };


/* Static variable only available and used in this file */
static uint8_t range = 0;


/* Prototypes for static methods only used by other methods in this file
 * (Not available to be used elsewhere) */
static void softResetADXL (void);
static bool checkID_ADXL (void);
static int32_t convertGRangeToGValue (int8_t sensorValue);


/**************************************************************************//**
 * @brief
 *   Initialize the GPIO pin to supply the accelerometer with power.
 *****************************************************************************/
void initADXL_VCC (void)
{
	/* In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1". */
	GPIO_PinModeSet(ADXL_VDD_PORT, ADXL_VDD_PIN, gpioModePushPull, 1);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("ADXL362 VDD pin initialized and enabled");
#endif /* DEBUGGING */

}

/**************************************************************************//**
 * @brief
 *   Enable or disable the power to the accelerometer.
 *
 * @param[in] enabled
 *   @li True - Enable the GPIO pin connected to the VDD pin of the accelerometer.
 *   @li False - Disable the GPIO pin connected to the VDD pin of the accelerometer.
 *****************************************************************************/
void powerADXL (bool enabled)
{
	if (enabled)
	{
		GPIO_PinOutSet(ADXL_VDD_PORT, ADXL_VDD_PIN); /* Enable VDD pin */

#ifdef DEBUGGING /* DEBUGGING */
		dbinfo("Accelerometer powered");
#endif /* DEBUGGING */

	}
	else
	{
		GPIO_PinOutClear(ADXL_VDD_PORT, ADXL_VDD_PIN); /* Disable VDD pin */

#ifdef DEBUGGING /* DEBUGGING */
		dbwarn("Accelerometer powered down");
#endif /* DEBUGGING */

	}
}


/**************************************************************************//**
 * @brief
 *   Enable or disable the SPI pins to the accelerometer.
 *
 * @param[in] enabled
 *   @li True - Enable the SPI pins to the accelerometer.
 *   @li False - Disable the SPI pins to the accelerometer.
 *****************************************************************************/
void enableSPIpinsADXL (bool enabled)
{
	if (enabled)
	{
		/* In the case of gpioModePushPull", the last argument directly sets the
		 * the pin low if the value is "0" or high if the value is "1". */
		GPIO_PinModeSet(ADXL_CLK_PORT, ADXL_CLK_PIN, gpioModePushPull, 0);   /* US0_CLK is push pull */
		GPIO_PinModeSet(ADXL_NCS_PORT, ADXL_NCS_PIN, gpioModePushPull, 1);   /* US0_CS is push pull */
		GPIO_PinModeSet(ADXL_MOSI_PORT, ADXL_MOSI_PIN, gpioModePushPull, 1); /* US0_TX (MOSI) is push pull */
		GPIO_PinModeSet(ADXL_MISO_PORT, ADXL_MISO_PIN, gpioModeInput, 1);    /* US0_RX (MISO) is input */
	}
	else
	{
		/* gpioModeDisabled: Pull-up if DOUT is set. */
		GPIO_PinModeSet(ADXL_CLK_PORT, ADXL_CLK_PIN, gpioModeDisabled, 0);
		GPIO_PinModeSet(ADXL_NCS_PORT, ADXL_NCS_PIN, gpioModeDisabled, 1);
		GPIO_PinModeSet(ADXL_MOSI_PORT, ADXL_MOSI_PIN, gpioModeDisabled, 1);
		GPIO_PinModeSet(ADXL_MISO_PORT, ADXL_MISO_PIN, gpioModeDisabled, 1);
	}
}


/**************************************************************************//**
 * @brief
 *   Initialize USART0 in SPI mode according to the settings required
 *   by the accelerometer.
 *
 * @details
 *   Enable clocks, configure pins, configure USART0 in SPI mode, route
 *   the pins, enable USART0 and set CS high.
 *****************************************************************************/
void initADXL_SPI (void)
{
	/* Enable necessary clocks */
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART0, true);

	/* Configure GPIO
	 * In the case of gpioModePushPull", the last argument directly sets the
	 * the pin low if the value is "0" or high if the value is "1". */
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

	/* Initialize USART0 with the configured parameters */
	USART_InitSync(USART0, &config);

	/* Set the pin location for USART0 */
	USART0->ROUTE = USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC0;

	/* Enable USART0 */
	USART_Enable(USART0, usartEnable);

	/* Set CS high (active low!) */
	GPIO_PinOutSet(gpioPortE, 13);

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("Accelerometer SPI initialized");
#endif /* DEBUGGING */

}


/**************************************************************************//**
 * @brief
 *   Go through all of the register settings to see the influence
 *   they have on power usage.
 *****************************************************************************/
void testADXL (void)
{

#ifdef DEBUGGING /* DEBUGGING */
	dbwarn("Waiting 5 seconds...");
#endif /* DEBUGGING */

	delay(5000);

#ifdef DEBUGGING /* DEBUGGING */
	dbwarn("Starting...");
#endif /* DEBUGGING */

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("Testing the ADXL (all in +-2g default measurement range, 7 seconds):");
	dbprintln("   standby - 1sec - ODR 12,5Hz + enable measurements - 1sec - ODR 25 Hz");
	dbprintln("   - 1sec - ODR 50 Hz - 1sec - ODR 100 Hz (default on reset) - 1sec -");
	dbprintln("   200Hz - 1sec - ODR 400 Hz - 1sec - soft Reset");
#endif /* DEBUGGING */

	/* Soft reset ADXL */
	softResetADXL();

	/* Standby */
	delay(1000);

	/* ODR 12,5Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010000); /* last 3 bits are ODR */

	/* Enable measurements */
	writeADXL(ADXL_REG_POWER_CTL, 0b00000010); /* last 2 bits are measurement mode */

	delay(1000);

	/* ODR 25Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010001); /* last 3 bits are ODR */
	delay(1000);

	/* ODR 50Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010010); /* last 3 bits are ODR */
	delay(1000);

	/* ODR 100Hz (default) */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010011); /* last 3 bits are ODR */
	delay(1000);

	/* ODR 200Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010100); /* last 3 bits are ODR */
	delay(1000);

	/* ODR 400Hz */
	writeADXL(ADXL_REG_FILTER_CTL, 0b00010101); /* last 3 bits are ODR */
	delay(1000);

	/* Soft reset ADXL */
	softResetADXL();

#ifdef DEBUGGING /* DEBUGGING */
	dbinfo("Testing done");
#endif /* DEBUGGING */

}


/**************************************************************************//**
 * @brief
 *   Read and display g values forever with a 100ms interval.
 *
 * @details
 *   The accelerometer is put in measurement mode at 12.5Hz ODR, new
 *   values are displayed every 100ms, if an interrupt was generated
 *   a delay of one second is called.
 *****************************************************************************/
void readValuesADXL (void)
{
	uint32_t counter = 0;

	/* Enable measurement mode */
	measureADXL(true);

	/* Infinite loop */
	while (1)
	{
		led(true); /* Enable LED */

		/* Read XYZ sensor data */
		readADXL_XYZDATA();

#ifdef DEBUGGING /* DEBUGGING */
		/* Print XYZ sensor data */
		//dbprint("[");
		dbprint("\r[");
		dbprintInt(counter);
		dbprint("] X: ");
		dbprintInt(convertGRangeToGValue(XYZDATA[0]));
		dbprint(" mg | Y: ");
		dbprintInt(convertGRangeToGValue(XYZDATA[1]));
		dbprint(" mg | Z: ");
		dbprintInt(convertGRangeToGValue(XYZDATA[2]));
		dbprint(" mg   "); /* Extra spacing is to overwrite other data if it's remaining (see \r) */
		//dbprintln("");
#endif /* DEBUGGING */

		led(false); /* Disable LED */

		counter++;

		delay(100);

		/* Read status register to acknowledge interrupt
		 * (can be disabled by changing LINK/LOOP mode in ADXL_REG_ACT_INACT_CTL) */
		if (triggered)
		{
			delay(1000);

			readADXL(ADXL_REG_STATUS);
			triggered = false;
		}
	}
}


/**************************************************************************//**
 * @brief
 *   Soft reset accelerometer handler.
 *
 * @details
 *   If the first ID check fails, the MCU is put on hold for one second
 *   and the ID gets checked again.
 *****************************************************************************/
void resetHandlerADXL (void)
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

				delay(1000);

				powerADXL(true);

				delay(1000);

				/* Soft reset */
				softResetADXL();

				/* Last try to get the correct ID failed */
				if (!checkID_ADXL())
				{
					error(1);
				}
			}
		}
	}

#ifdef DEBUGGING /* DEBUGGING */
	if (retries < 2) dbinfoInt("Soft reset ADXL done (", retries, " retries)");
	else dbwarnInt("Soft reset ADXL done, had to \"hard reset\" (", retries, " retries)");
#endif /* DEBUGGING */

}


/**************************************************************************//**
 * @brief
 *   Read an SPI byte from the accelerometer (8 bits) using a given address.
 *
 * @param[in] address
 *   The register address to read from.
 *
 * @return
 *   The response (one byte, uint8_t).
 *****************************************************************************/
uint8_t readADXL (uint8_t address) /* TODO: Make this static */
{
	uint8_t response;

	/* Set CS low (active low!) */
	GPIO_PinOutClear(ADXL_NCS_PORT, ADXL_NCS_PIN);

	/* 3-byte operation according to datasheet */
	USART_SpiTransfer(USART0, 0x0B);            /* "read" instruction */
	USART_SpiTransfer(USART0, address);         /* Address */
	response = USART_SpiTransfer(USART0, 0x00); /* Read response */

	/* Set CS high */
	GPIO_PinOutSet(ADXL_NCS_PORT, ADXL_NCS_PIN);

	return (response);
}


/**************************************************************************//**
 * @brief
 *   Write an SPI byte to the accelerometer (8 bits) using a given address
 *   and specified data.
 *
 * @param[in] address
 *   The register address to write the data to (one byte, uint8_t).
 *
 * @param[in] data
 *   The data to write to the address (one byte, uint8_t).
 *****************************************************************************/
void writeADXL (uint8_t address, uint8_t data) /* TODO: Make this static */
{
	/* Set CS low (active low!) */
	GPIO_PinOutClear(ADXL_NCS_PORT, ADXL_NCS_PIN);

	/* 3-byte operation according to datasheet */
	USART_SpiTransfer(USART0, 0x0A);    /* "write" instruction */
	USART_SpiTransfer(USART0, address); /* Address */
	USART_SpiTransfer(USART0, data);    /* Data */

	/* Set CS high */
	GPIO_PinOutSet(ADXL_NCS_PORT, ADXL_NCS_PIN);
}


/**************************************************************************//**
 * @brief
 *   Read the X-Y-Z data registers in the XYZDATA[] field using burst reads.
 *
 * @details
 *   Response data gets put in XYZDATA array (global volatile variable).
 *****************************************************************************/
void readADXL_XYZDATA (void)
{
	/* CS low (active low!) */
	GPIO_PinOutClear(ADXL_NCS_PORT, ADXL_NCS_PIN);

	/* Burst read (address auto-increments) */
	USART_SpiTransfer(USART0, 0x0B);				/* "read" instruction */
	USART_SpiTransfer(USART0, ADXL_REG_XDATA); 		/* Address */
	XYZDATA[0] = USART_SpiTransfer(USART0, 0x00);	/* Read response */
	XYZDATA[1] = USART_SpiTransfer(USART0, 0x00);	/* Read response */
	XYZDATA[2] = USART_SpiTransfer(USART0, 0x00);	/* Read response */

	/* CS high */
	GPIO_PinOutSet(ADXL_NCS_PORT, ADXL_NCS_PIN);
}


/**************************************************************************//**
 * @brief
 *   Configure the Output Data Rate (ODR).
 *
 * @param[in] givenODR
 *   @li 0 - 12.5 Hz
 *   @li 1 - 25 Hz
 *   @li 2 - 50 Hz
 *   @li 3 - 100 Hz (reset default)
 *   @li 4 - 200 Hz
 *   @li 5 - 400 Hz
 *****************************************************************************/
void configADXL_ODR (uint8_t givenODR)
{
	/* Get value in register */
	uint8_t reg = readADXL(ADXL_REG_FILTER_CTL);

	/* AND with mask to keep the bits we don't want to change */
	reg = reg & 0b11111000;

	/* OR with new setting bits */

	/* Set ODR (last three bits) */
	if (givenODR == 0) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000000));
	if (givenODR == 1) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000001));
	if (givenODR == 2) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000010));
	if (givenODR == 3) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000011));
	if (givenODR == 4) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000100));
	if (givenODR == 5) writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000101));
	else writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000011));

#ifdef DEBUGGING /* DEBUGGING */
	if (givenODR == 0) dbinfo("ODR set at 12.5 Hz");
	else if (givenODR == 1) dbinfo("ODR set at 25 Hz");
	else if (givenODR == 2) dbinfo("ODR set at 50 Hz");
	else if (givenODR == 3) dbinfo("ODR set at 100 Hz");
	else if (givenODR == 4) dbinfo("ODR set at 200 Hz");
	else if (givenODR == 5) dbinfo("ODR set at 400 Hz");
	else dbinfo("ODR set at 100 Hz (reset default)");
#endif /* DEBUGGING */

}


/**************************************************************************//**
 * @brief
 *   Configure the measurement range and store the selected one in
 *   a global variable.
 *
 * @param[in] givenRange
 *   @li 0 - +- 2g
 *   @li 1 - +- 4g
 *   @li 2 - +- 8g
 *****************************************************************************/
void configADXL_range (uint8_t givenRange)
{
	/* Get value in register */
	uint8_t reg = readADXL(ADXL_REG_FILTER_CTL);

	/* AND with mask to keep the bits we don't want to change */
	reg = reg & 0b00111111;

	/* OR with new setting bits */

	/* Set measurement range (first two bits) */
	if (givenRange == 0) {
		writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b00000000));
		range = 0;
	}
	else if (givenRange == 1) {
		writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b01000000));
		range = 1;
	}
	else if (givenRange == 2) {
		writeADXL(ADXL_REG_FILTER_CTL, (reg | 0b10000000));
		range = 2;
	}

#ifdef DEBUGGING /* DEBUGGING */
	if (range == 0) dbinfo("Measurement mode +- 2g selected");
	else if (range == 1) dbinfo("Measurement mode +- 4g selected");
	else if (range == 2) dbinfo("Measurement mode +- 8g selected");
#endif /* DEBUGGING */

}


/**************************************************************************//**
 * @brief
 *   Configure the accelerometer to work in activity threshold mode.
 *
 * @details
 *   Route activity detector to INT1 pin using INTMAP1, isolate bits
 *   and write settings to both threshold registers.
 *
 * @param[in] gThreshold
 *   Threshold [g].
 *****************************************************************************/
void configADXL_activity (uint8_t gThreshold)
{
	/* Map activity detector to INT1 pin  */
	writeADXL(ADXL_REG_INTMAP1, 0b00010000); /* Bit 4 selects activity detector */

	/* Enable referenced activity threshold mode (last two bits) */
	writeADXL(ADXL_REG_ACT_INACT_CTL, 0b00000011);

	/* Convert g value to "codes":
	 * THRESH_ACT [codes] = Threshold Value [g] × Scale Factor [LSB per g] */
	uint16_t threshold;

	if (range == 0) threshold = gThreshold * 1000;
	else if (range == 1) threshold = gThreshold * 500;
	else if (range == 2) threshold = gThreshold * 250;
	else threshold = 0;

	/* Isolate bits using masks and shifting */
	uint8_t low  = (threshold & 0b0011111111);
	uint8_t high = (threshold & 0b1100000000) >> 8;

	/* Set threshold register values (total: 11bit unsigned) */
	writeADXL(ADXL_REG_THRESH_ACT_L, low);  /* 7:0 bits used */
	writeADXL(ADXL_REG_THRESH_ACT_H, high); /* 2:0 bits used */

#ifdef DEBUGGING /* DEBUGGING */
	dbinfoInt("Activity configured: ", gThreshold, " g");
#endif /* DEBUGGING */

}

/**************************************************************************//**
 * @brief
 *   Enable or disable measurement mode.
 *
 * @param[in] enabled
 *   @li True - Enable measurement mode.
 *   @li False - Disable measurement mode (standby).
 *****************************************************************************/
void measureADXL (bool enabled)
{
	if (enabled)
	{
		/* Get value in register */
		uint8_t reg = readADXL(ADXL_REG_POWER_CTL);

		/* AND with mask to keep the bits we don't want to change */
		reg = reg & 0b11111100;

		/* OR with new setting bits */

		/* Enable measurements */
		writeADXL(ADXL_REG_POWER_CTL, reg | 0b00000010); /* Last 2 bits are measurement mode */

#ifdef DEBUGGING /* DEBUGGING */
		dbinfo("Measurement enabled");
#endif /* DEBUGGING */

	}
	else
	{
		/* Get value in register */
		uint8_t reg = readADXL(ADXL_REG_POWER_CTL);

		/* AND with mask to keep the bits we don't want to change */
		reg = reg & 0b11111100;

		/* OR with new setting bits */

		/* Enable measurements */
		writeADXL(ADXL_REG_POWER_CTL, reg | 0b00000000); /* Last 2 bits are measurement mode */

#ifdef DEBUGGING /* DEBUGGING */
		dbinfo("Measurement disabled (standby)");
#endif /* DEBUGGING */
	}
}


/**************************************************************************//**
 * @brief
 *   Soft reset accelerometer.
 *
 * @note
 *   This is a static (~hidden) method because it's only internally used
 *   in this file and called by other methods if necessary.
 *****************************************************************************/
static void softResetADXL (void)
{
	writeADXL(ADXL_REG_SOFT_RESET, 0x52); /* 0x52 = "R" */
}


/**************************************************************************//**
 * @brief Check if the ID is correct.
 *
 * @note
 *   This is a static (~hidden) method because it's only internally used
 *   in this file and called by other methods if necessary.
 *
 * @return
 *   @li true - Correct ID returned.
 *   @li false - Incorrect ID returned.
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
 *   This is a static (~hidden) method because it's only internally used
 *   in this file and called by other methods if necessary.
 *
 * @param[in] sensorValue
 *   Value in g-range returned by sensor.
 *
 * @return
 *   The calculated mg value.
 *****************************************************************************/
static int32_t convertGRangeToGValue (int8_t sensorValue)
{
	/* 255 = (-) 128 + 127 */

	/* 2 = + & -
	 * 1000 = "m"g */

	if (range == 0) return ((2*2*1000 / 255) * sensorValue);
	else if (range == 1) return ((2*4*1000 / 255) * sensorValue);
	else if (range == 2) return ((2*8*1000 / 255) * sensorValue);
	else return (0);
}
