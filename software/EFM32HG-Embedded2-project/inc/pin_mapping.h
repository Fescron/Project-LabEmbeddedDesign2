/***************************************************************************//**
 * @file pin_mapping.h
 * @brief The pin definitions for the regular and custom Happy Gecko board.
 * @version 2.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Started with custom board pinout.
 *   @li v1.1: Added regular board pinout, selectable with define statement.
 *   @li v1.2: Added documentation.
 *   @li v1.3: Updated code with new DEFINE checks.
 *   @li v1.4: Added IIC definitions.
 *   @li v2.0: Updated version number.
 *
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _PIN_MAPPING_H_
#define _PIN_MAPPING_H_


/* Include necessary for this header file */
#include "em_device.h" /* Include necessary MCU-specific header file */
#include "em_gpio.h"   /* General Purpose IO */


/** Public definition to select the Happy Gecko board *version*
 *    @li `1` - Use the custom Happy Gecko board pinout.
 *    @li `0` - Use the regular Happy Gecko board pinout. */
#define CUSTOM_BOARD 1


#if CUSTOM_BOARD == 1 /* Custom Happy Gecko pinout */

	/* ADXL362 */
	#define ADXL_SPI            USART0
	#define ADXL_SPI_LOC        0
	#define ADXL_MOSI_PORT      gpioPortE
	#define ADXL_MOSI_PIN       10
	#define ADXL_MISO_PORT      gpioPortE
	#define ADXL_MISO_PIN       11
	#define ADXL_CLK_PORT       gpioPortE
	#define ADXL_CLK_PIN        12
	#define ADXL_NCS_PORT       gpioPortF /* Can't use the US0_CS port (PE13) to manually set/clear CS line */
	#define ADXL_NCS_PIN        5

	#define ADXL_INT1_PORT      gpioPortF
	#define ADXL_INT1_PIN       3
	#define ADXL_INT2_PORT      gpioPortF /* Unused */
	#define ADXL_INT2_PIN       4         /* Unused */
	#define ADXL_VDD_PORT       gpioPortA
	#define ADXL_VDD_PIN        1

	/* LED */
	#define LED_PORT            gpioPortA
	#define LED_PIN             2

	/* Buttons */
	#define PB0_PORT            gpioPortC
	#define PB0_PIN             9
	#define PB1_PORT            gpioPortC
	#define PB1_PIN             10

	/* UART */
	#define DBG_UART            USART1
	#define DBG_UART_LOC        4
	#define DBG_TXD_PORT        gpioPortF
	#define DBG_TXD_PIN         2
	#define DBG_RXD_PORT        gpioPortA
	#define DBG_RXD_PIN         0

	/* DS18B20 */
	#define TEMP_DATA_PORT      gpioPortC
	#define TEMP_DATA_PIN       4
	#define TEMP_VDD_PORT       gpioPortB
	#define TEMP_VDD_PIN        11

	/* Link breakage sensor */
	#define BREAK1_PORT         gpioPortC
	#define BREAK1_PIN          2
	#define BREAK2_PORT         gpioPortC
	#define BREAK2_PIN          3

	/* RN2483 */
	#define RN2483_UART         LEUART0
	#define RN2483_UART_LOC     0
	#define RN2483_TX_PORT      gpioPortD
	#define RN2483_TX_PIN       4
	#define RN2483_RX_PORT      gpioPortD
	#define RN2483_RX_PIN       5
	#define RN2483_RESET_PORT   gpioPortA
	#define RN2483_RESET_PIN    10

	/* External header */
	#define IIC_EXT             I2C0      /* Unused */
	#define IIT_EXT_LOC         4         /* Unused */
	#define IIC_SDA_PORT        gpioPortC /* Unused */
	#define IIC_SDA_PIN         0         /* Unused */
	#define IIC_SCL_PORT        gpioPortC /* Unused */
	#define IIC_SCL_PIN         1         /* Unused */

	/* Power supply enable */
	#define PM_RN2483_PORT		gpioPortA
	#define PM_RN2483_PIN		8
	#define PM_SENS_EXT_PORT	gpioPortA
	#define PM_SENS_EXT_PIN		9

#else /* Regular Happy Gecko pinout */

	/* ADXL362 */
	#define ADXL_SPI            USART0
	#define ADXL_SPI_LOC        0
	#define ADXL_MOSI_PORT      gpioPortE
	#define ADXL_MOSI_PIN       10
	#define ADXL_MISO_PORT      gpioPortE
	#define ADXL_MISO_PIN       11
	#define ADXL_CLK_PORT       gpioPortE
	#define ADXL_CLK_PIN        12
	#define ADXL_NCS_PORT       gpioPortD 	/* Can't use the US0_CS port (PE13) to manually set/clear CS line */
	#define ADXL_NCS_PIN        4

	#define ADXL_INT1_PORT      gpioPortD
	#define ADXL_INT1_PIN       7
	// #define ADXL_INT2_PORT      gpioPortF
	// #define ADXL_INT2_PIN       4
	#define ADXL_VDD_PORT       gpioPortD
	#define ADXL_VDD_PIN        5

	/* LED */
	#define LED_PORT            gpioPortF
	#define LED_PIN             4

	/* Buttons */
	#define PB0_PORT            gpioPortC
	#define PB0_PIN             9
	#define PB1_PORT            gpioPortC
	#define PB1_PIN             10

	/* UART */
	#define DBG_UART            USART1
	#define DBG_UART_LOC        4
	#define DBG_TXD_PORT        gpioPortF
	#define DBG_TXD_PIN         2
	#define DBG_RXD_PORT        gpioPortA
	#define DBG_RXD_PIN         0

	/* DS18B20 */
	#define TEMP_DATA_PORT      gpioPortA
	#define TEMP_DATA_PIN       1
	#define TEMP_VDD_PORT       gpioPortA
	#define TEMP_VDD_PIN        2

	/* Link breakage sensor */
	#define BREAK1_PORT         gpioPortC
	#define BREAK1_PIN          1
	#define BREAK2_PORT         gpioPortC
	#define BREAK2_PIN          2

	/* RN2483 */
	#define RN2483_UART         LEUART0
	#define RN2483_UART_LOC     0
	#define RN2483_TX_PORT      gpioPortD
	#define RN2483_TX_PIN       4
	#define RN2483_RX_PORT      gpioPortD
	#define RN2483_RX_PIN       5
	#define RN2483_RESET_PORT   gpioPortA
	#define RN2483_RESET_PIN    10

	/* External header */
	#define IIC_EXT             I2C0      /* Unused */
	#define IIT_EXT_LOC         4         /* Unused */
	#define IIC_SDA_PORT        gpioPortC /* Unused */
	#define IIC_SDA_PIN         0         /* Unused */
	#define IIC_SCL_PORT        gpioPortC /* Unused */
	#define IIC_SCL_PIN         1         /* Unused */

	/* Power supply enable */
	#define PM_RN2483_PORT		gpioPortA
	#define PM_RN2483_PIN		8
	#define PM_SENS_EXT_PORT	gpioPortA
	#define PM_SENS_EXT_PIN		9

#endif /* Board pinout selection */


#endif /* _PIN_MAPPING_H_ */
