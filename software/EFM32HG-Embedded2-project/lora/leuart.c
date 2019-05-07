/***************************************************************************//**
 * @file leuart.c
 * @brief LEUART (serial communication) functionality required by the RN2483 LoRa modem.
 * @version 2.0
 * @author
 *   Guus Leenders@n
 *   Modified by Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: DRAMCO GitHub version (https://github.com/DRAMCO/EFM32-RN2483-LoRa-Node).
 *   @li v1.1: Removed rtcdriver functionality (timeouts)
 *   @li v1.2: Added (basic) timeout functionality.
 *   @li v1.3: Refined timeout functionality and cleaned up unused things in comments.
 *   @li v2.0: Updated version number.
 *
 ******************************************************************************/

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
 *         File: leuart.c
 *      Created: 2018-01-26
 *       Author: Guus Leenders - Modified by Brecht Van Eeckhoudt
 *
 *  Description: This file contains the LEUART (serial communication)
 *  	functionality required by the RN2483 LoRa modem.
 */


#include <stdint.h>        /* (u)intXX_t */
#include <stdbool.h>       /* "bool", "true", "false" */
#include "em_device.h"     /* Include necessary MCU-specific header file */
#include "em_chip.h"       /* Chip Initialization */
#include "em_cmu.h"        /* Clock management unit */
#include "em_gpio.h"       /* General Purpose IO */
#include "em_leuart.h"     /* Low Energy Universal Asynchronous Receiver/Transmitter Peripheral API */
#include "em_dma.h"        /* Direct Memory Access (DMA) API */
#include "dmactrl.h"       /* DMA driver */

#include "leuart.h"        /* Corresponding header file */
#include "delay.h"         /* Delay functionality */
#include "pin_mapping.h"   /* PORT and PIN definitions */
#include "debug_dbprint.h" /* Enable or disable printing to UART for debugging */
#include "util_string.h"   /* Utility functionality regarding strings */
#include "util.h"          /* Utility functionality */


/* Local definitions */
/** Enable (1) or disable (0) printing the timeout counter value using DBPRINT */
#define DBPRINT_TIMEOUT 0

/* Maximum values for the counters before exiting a `while` loop */
#define TIMEOUT_SYNC         80
#define TIMEOUT_DMA          50000
#define TIMEOUT_SENDCMD      40000
#define TIMEOUT_WAITRESPONSE 2000000 /* Depends on spreading factor! */

/* DMA Configurations */
#define DMA_CHANNEL_TX       0 /* DMA channel is 0 */
#define DMA_CHANNEL_RX       1
#define DMA_CHANNELS 		 2


/* Local variables */
static DMA_CB_TypeDef dmaCallBack[DMA_CHANNELS]; /* DMA callback structure */
char receiveBuffer[RECEIVE_BUFFER_SIZE];
volatile uint8_t bufferPointer = 0;
volatile bool receiveComplete = false;


void Leuart_ClearBuffers(void)
{
	memset(receiveBuffer, '\0', RECEIVE_BUFFER_SIZE);
	receiveComplete = false;
}

/* Static (internal) functions */
static void basicTxComplete(unsigned int channel, bool primary, void *user)
{
	(void) user;
	/* Refresh DMA basic transaction cycle */
	DMA_ActivateBasic(DMA_CHANNEL_RX,
					primary,
					false,
					(void *)&receiveBuffer[0],
					(void *)&RN2483_UART->RXDATA,
					0);
	bufferPointer = 0;
}

static void basicRxComplete(unsigned int channel, bool primary, void *user)
{
	(void) user;

	/* Refresh DMA basic transaction cycle */
	char c = RN2483_UART->RXDATA;
	if(bufferPointer < RECEIVE_BUFFER_SIZE - 1){
		if(c != '\n'){
			bufferPointer++;
			DMA_ActivateBasic(DMA_CHANNEL_RX,
								  primary,
								  false,
								  &receiveBuffer[bufferPointer],
								  NULL,
								  0);
		}
		else{
			receiveComplete = true;
			receiveBuffer[bufferPointer+1] = '\0';
		}
	}
}

static bool Leuart_ResponseAvailable(void){
	return (receiveComplete);
}

void setupDma(void){
	/* DMA configuration structs */
	DMA_Init_TypeDef       dmaInit;
	DMA_CfgChannel_TypeDef rxChnlCfg;
	DMA_CfgChannel_TypeDef txChnlCfg;
	DMA_CfgDescr_TypeDef   rxDescrCfg;
	DMA_CfgDescr_TypeDef   txDescrCfg;

	/* Initializing the DMA */
	dmaInit.hprot        = 0;
	dmaInit.controlBlock = dmaControlBlock;
	DMA_Init(&dmaInit);

	/* RX DMA setup*/
	/* Set the interrupt callback routine */
	dmaCallBack[DMA_CHANNEL_RX].cbFunc = basicRxComplete;
	/* Callback doesn't need userpointer */
	dmaCallBack[DMA_CHANNEL_RX].userPtr = NULL;

	/* Setting up channel for TX*/
	rxChnlCfg.highPri   = false; /* Can't use with peripherals */
	rxChnlCfg.enableInt = true;  /* Enabling interrupt to refresh DMA cycle*/
	/*Setting up DMA transfer trigger request*/
	rxChnlCfg.select = DMAREQ_LEUART0_RXDATAV; /* DMAREQ_LEUART0_RXDATAV; */
	/* Setting up callback function to refresh descriptors*/
	rxChnlCfg.cb     = &(dmaCallBack[DMA_CHANNEL_RX]);
	DMA_CfgChannel(DMA_CHANNEL_RX, &rxChnlCfg);

	/* Setting up channel descriptor */
	/* Destination is buffer, increment ourselves */
	rxDescrCfg.dstInc = dmaDataIncNone;
	/* Source is LEUART_RX register and transfers 8 bits each time */
	rxDescrCfg.srcInc = dmaDataIncNone;
	rxDescrCfg.size   = dmaDataSize1;
	/* Default setting of DMA arbitration*/
	rxDescrCfg.arbRate = dmaArbitrate1;
	rxDescrCfg.hprot   = 0;
	/* Configure primary descriptor */
	DMA_CfgDescr(DMA_CHANNEL_RX, true, &rxDescrCfg);
	DMA_CfgDescr(DMA_CHANNEL_RX, false, &rxDescrCfg);

	/* TX DMA setup*/
	/* Set the interrupt callback routine */
	dmaCallBack[DMA_CHANNEL_TX].cbFunc = basicTxComplete;
	/* Callback doesn't need userpointer */
	dmaCallBack[DMA_CHANNEL_TX].userPtr = NULL;

	/* Setting up channel for TX*/
	txChnlCfg.highPri   = false; /* Can't use with peripherals */
	txChnlCfg.enableInt = true;  /* Enabling interrupt to refresh DMA cycle*/
	/*Setting up DMA transfer trigger request*/
	txChnlCfg.select = DMAREQ_LEUART0_TXBL; /* DMAREQ_LEUART0_RXDATAV; */
	/* Setting up callback function to refresh descriptors*/
	txChnlCfg.cb     = &(dmaCallBack[DMA_CHANNEL_TX]);
	DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

	/* Setting up channel descriptor */
	/* Destination is LEUART_Tx register and doesn't move */
	txDescrCfg.dstInc = dmaDataIncNone;
	/* Source is LEUART_TX register and transfers 8 bits each time */
	txDescrCfg.srcInc = dmaDataInc1;
	txDescrCfg.size   = dmaDataSize1;
	/* Default setting of DMA arbitration*/
	txDescrCfg.arbRate = dmaArbitrate1;
	txDescrCfg.hprot   = 0;
	/* Configure primary descriptor */
	DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);
}

static void sendLeuartData(char * buffer, uint8_t bufferLength)
{
	/* Timeout counter */
	uint32_t counter = 0;

	/* Wait for sync */
	while ((counter < TIMEOUT_SYNC) && RN2483_UART->SYNCBUSY) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_SYNC)
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Waiting time for sync reached! (sendLeuartData)");
#endif  /* DEBUG_DBPRINT */

		error(51);
	}
#if DBPRINT_TIMEOUT == 1 /* DBPRINT_TIMEOUT */
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbwarnInt("sendLeuartData SYNC (", counter, ")");
#endif /* DEBUG_DBPRINT */

	}
#endif /* DBPRINT_TIMEOUT */

	DMA_ActivateBasic(DMA_CHANNEL_TX,
	                  true,
	                  false,
	                  (void *)&RN2483_UART->TXDATA,
	                  buffer,
	                  (unsigned int)(bufferLength - 1));

	/* Reset timeout counter */
	counter = 0;

	/* Wait in EM1 for DMA channel enable */
	while ((counter < TIMEOUT_DMA) && DMA_ChannelEnabled(DMA_CHANNEL_TX)) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_DMA)
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Waiting time for DMA channel enable reached! (sendLeuartData)");
#endif /* DEBUG_DBPRINT */

		error(52);
	}
#if DBPRINT_TIMEOUT == 1 /* DBPRINT_TIMEOUT */
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbwarnInt("sendLeuartData DMA (", counter, ")");
#endif /* DEBUG_DBPRINT */

	}
#endif /* DBPRINT_TIMEOUT */

}

static void setupLeuart(void)
{
	/* Enable peripheral clocks */
	CMU_ClockEnable(cmuClock_HFPER, true);
	/* Configure GPIO pins */
	CMU_ClockEnable(cmuClock_GPIO, true);
	/* To avoid false start, configure output as high */
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(RN2483_RX_PORT, RN2483_RX_PIN, gpioModeInput, 0);

	LEUART_Init_TypeDef init = LEUART_INIT_DEFAULT; /* Default config is fine */
	init.baudrate = 4800;

	/* Enable CORE LE clock in order to access LE modules */
	CMU_ClockEnable(cmuClock_CORELE, true);

	/* Select LFRCO for LEUARTs (and wait for it to stabilize) */
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_LEUART0, true);

	/* Do not prescale clock */
	CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1);

	/* Configure LEUART */
	init.enable = leuartDisable;

	LEUART_Init(RN2483_UART, &init);

	/* Enable pins at default location */
	RN2483_UART->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | RN2483_UART_LOC;

	/* Set RXDMAWU to wake up the DMA controller in EM2 */
	LEUART_RxDmaInEM2Enable(RN2483_UART, true);

	/* Clear previous RX interrupts */
	LEUART_IntClear(RN2483_UART, LEUART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(LEUART0_IRQn);

	/* Finally enable it */
	LEUART_Enable(RN2483_UART, leuartEnable);
}

void Leuart_Init(void)
{
	// TODO: Lines below might fix some sleep functionality...
	//CMU_ClockEnable(cmuClock_CORELE, true);
	//CMU_ClockEnable(cmuClock_DMA, true);
	//CMU_ClockEnable(cmuClock_LEUART0, true);

	setupDma();
	Leuart_BreakCondition();
	setupLeuart();

	/* Auto baud setting */
	char b[] = "U";
	sendLeuartData(b, 1);
	delay(500);
}

void Leuart_Reinit(void)
{
	// TODO: Lines below might fix some sleep functionality...
	//CMU_ClockEnable(cmuClock_CORELE, true);
	//CMU_ClockEnable(cmuClock_DMA, true);
	//CMU_ClockEnable(cmuClock_LEUART0, true);

	LEUART_Reset(RN2483_UART);
	Leuart_BreakCondition();
	setupLeuart();

	/* Auto baud setting */
	char b[] = "U";
	sendLeuartData(b, 1);
	Leuart_WaitForResponse();
	delay(20);
	Leuart_WaitForResponse();
}

void Leuart_BreakCondition(void)
{
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 1);
	delay(40);
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 0);
	delay(20);
	GPIO_PinOutSet(RN2483_TX_PORT, RN2483_TX_PIN);
}

void Leuart_ReadResponse(char * buffer, uint8_t bufferLength)
{
	sprintf(buffer, "%s", receiveBuffer);
	receiveComplete = false;
	bufferPointer = 0;
}

void Leuart_SendData(char * buffer, uint8_t bufferLength) /* TODO: Not used... */
{
	/* Timeout counter */
	uint32_t counter = 0;

	/* Send data over LEUART */
	sendLeuartData(buffer, bufferLength);

	/* Wait for response */
	while ((counter < TIMEOUT_WAITRESPONSE) && !Leuart_ResponseAvailable()) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_WAITRESPONSE)
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Waiting time for response reached! (Leuart_SendData)");
#endif /* DEBUG_DBPRINT */

		error(53);
	}
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbwarnInt("Leuart_SendData (", counter, ")");
#endif /* DEBUG_DBPRINT */

	}

	receiveComplete = true;
}

/** Send a command string over the LEUART. "wakeUp" IS NOT USED */
Leuart_Status_t Leuart_SendCommand(char * cb, uint8_t cbl, volatile bool * wakeUp)
{
	/* Timeout counter */
	uint32_t counter = 0;

	/* Send data over LEUART */
	sendLeuartData(cb, cbl);

	/* Wait for response */
	while ((counter < TIMEOUT_SENDCMD) && !Leuart_ResponseAvailable()) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_SENDCMD)
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Waiting time for response reached! (Leuart_SendCommand)");
#endif /* DEBUG_DBPRINT */

		error(54);

		return (TX_TIMEOUT);
	}
#if DBPRINT_TIMEOUT == 1 /* DBPRINT_TIMEOUT */
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbwarnInt("Leuart_SendCommand (", counter, ")");
#endif /* DEBUG_DBPRINT */

	}
#endif /* DBPRINT_TIMEOUT */

	return (DATA_SENT);
}


Leuart_Status_t Leuart_WaitForResponse()
{
	/* Timeout counter */
	uint32_t counter = 0;

	/* Activate DMA */
	DMA_ActivateBasic(	DMA_CHANNEL_RX,
						true,
						false,
						(void *)&receiveBuffer[0],
						(void *)&RN2483_UART->RXDATA,
						0);

	/* Wait for response */
	while ((counter < TIMEOUT_WAITRESPONSE) && !Leuart_ResponseAvailable()) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_WAITRESPONSE)
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbcrit("Waiting time for response reached! (Leuart_WaitForResponse)");
#endif /* DEBUG_DBPRINT */

		error(55);

		return (RX_TIMEOUT);
	}
#if DBPRINT_TIMEOUT == 1 /* DBPRINT_TIMEOUT */
	else
	{

#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
		dbwarnInt("Leuart_WaitForResponse (", counter, ")");
#endif /* DEBUG_DBPRINT */

	}
#endif /* DBPRINT_TIMEOUT */

	return (DATA_RECEIVED);
}

