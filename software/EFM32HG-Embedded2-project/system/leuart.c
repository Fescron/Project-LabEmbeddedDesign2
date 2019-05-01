/***************************************************************************//**
 * @file leuart.c
 * @brief LEUART (serial communication) functionality required by the RN2483 LoRa modem.
 * @version 1.2
 * @author
 *   Guus Leenders@n
 *   Modified by Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: DRAMCO GitHub version (https://github.com/DRAMCO/EFM32-RN2483-LoRa-Node).
 *   @li v1.1: Removed rtcdriver functionality (timouts)
 *   @li v1.2: Added (basic) timeout functionality.
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


#include <stdint.h>      /* (u)intXX_t */
#include <stdbool.h>     /* "bool", "true", "false" */
#include "em_device.h"   /* Include necessary MCU-specific header file */
#include "em_chip.h"     /* Chip Initialization */
#include "em_cmu.h"      /* Clock management unit */
#include "em_emu.h"      /* Energy Management Unit */
#include "em_gpio.h"     /* General Purpose IO */
#include "em_leuart.h"   /* Low Energy Universal Asynchronous Receiver/Transmitter Peripheral API */
#include "em_dma.h"      /* Direct Memory Access (DMA) API */

#include <dmactrl.h>
//#include <rtcdriver.h>

#include "leuart.h"      /* Corresponding header file */
#include "delay.h"       /* Delay functionality */
#include "pin_mapping.h" /* PORT and PIN definitions */
#include "util_string.h" /* Utility functionality regarding strings */
#include "debugging.h"   /* Enable or disable printing to UART for debugging */
#include "util.h"        /* Utility functionality */


/** Maximum waiting value before exiting a `while` loop */
#define TIMEOUT_COUNTER 2000000 /* 40 000 is not enough for "DMA Channel Enable", 800 000 is not enough for "Leuart_WaitForResponse" but 900 000 is @SF10 (1 000 000 just in case) */
                                /* @SF12 2 000 000 works ... */

#define TX_TIMEOUT_DURATION		2000	// ms
#define RX_TIMEOUT_DURATION		10000	// ms

/* DMA Configurations */
#define DMA_CHANNEL_TX       0          /* DMA channel is 0 */
#define DMA_CHANNEL_RX       1
#define DMA_CHANNELS 		 2

/** DMA callback structure */
static DMA_CB_TypeDef dmaCallBack[DMA_CHANNELS];

//char commandBuffer[COMMAND_BUFFER_SIZE];
char receiveBuffer[RECEIVE_BUFFER_SIZE];
volatile uint8_t bufferPointer = 0;
volatile bool receiveComplete = false;

/* RTCDRV_TimerID_t xTimerForTimeout; */
static volatile bool timeout = false;

void Leuart_ClearBuffers(void){
	memset(receiveBuffer, '\0', RECEIVE_BUFFER_SIZE);
	receiveComplete = false;
}

/* void timeoutCb(RTCDRV_TimerID_t id, void *user){
	timeout = true;
} */

/* Static (internal) functions */
static void basicTxComplete(unsigned int channel, bool primary, void *user){
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

static void basicRxComplete(unsigned int channel, bool primary, void *user){
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
	rxChnlCfg.select = DMAREQ_LEUART0_RXDATAV; //DMAREQ_LEUART0_RXDATAV;
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
	txChnlCfg.select = DMAREQ_LEUART0_TXBL; //DMAREQ_LEUART0_RXDATAV;
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
	while ((counter < TIMEOUT_COUNTER) && RN2483_UART->SYNCBUSY) counter++;

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_COUNTER)
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Waiting time for sync reached! (sendLeuartData)");
#endif /* DEBUGGING */

		error(51);
	}

	DMA_ActivateBasic(DMA_CHANNEL_TX,
	                  true,
	                  false,
	                  (void *)&RN2483_UART->TXDATA,
	                  buffer,
	                  (unsigned int)(bufferLength - 1));

	/* Reset timeout counter */
	counter = 0;

	/* Wait in EM1 for DMA channel enable */
	while ((counter < TIMEOUT_COUNTER) && DMA_ChannelEnabled(DMA_CHANNEL_TX)) counter++;//  EMU_EnterEM1() TODO: check if EM1 works here

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_COUNTER)
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Waiting time for DMA channel enable reached! (sendLeuartData)");
#endif /* DEBUGGING */

		error(52);
	}
}

static void setupLeuart(void){
	/* Allocate timer for Timeout detection*/
	/* RTCDRV_AllocateTimer(&xTimerForTimeout); */

	/* Enable peripheral clocks */
	CMU_ClockEnable(cmuClock_HFPER, true);
	/* Configure GPIO pins */
	CMU_ClockEnable(cmuClock_GPIO, true);
	/* To avoid false start, configure output as high */
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(RN2483_RX_PORT, RN2483_RX_PIN, gpioModeInput, 0);

	LEUART_Init_TypeDef init = LEUART_INIT_DEFAULT; // Default config is fine
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

void Leuart_Init(void){
	// Brecht: added 3 lines below
	//CMU_ClockEnable(cmuClock_CORELE, true);
	//CMU_ClockEnable(cmuClock_DMA, true);
	//CMU_ClockEnable(cmuClock_LEUART0, true);

	setupDma();
	Leuart_BreakCondition();
	setupLeuart();

	// Auto baud setting
	char b[] = "U";
	sendLeuartData(b, 1);
	delay(500);
}

void Leuart_Reinit(void){
	// Brecht: added 3 lines below
	//CMU_ClockEnable(cmuClock_CORELE, true);
	//CMU_ClockEnable(cmuClock_DMA, true);
	//CMU_ClockEnable(cmuClock_LEUART0, true);

	LEUART_Reset(RN2483_UART);
	Leuart_BreakCondition();
	setupLeuart();

	// Auto baud setting
	char b[] = "U";
	sendLeuartData(b, 1);
	Leuart_WaitForResponse();
	delay(20);
	Leuart_WaitForResponse();
}

void Leuart_BreakCondition(void){
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 1);
	delay(40);
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 0);
	delay(20);
	GPIO_PinOutSet(RN2483_TX_PORT, RN2483_TX_PIN);
}

void Leuart_ReadResponse(char * buffer, uint8_t bufferLength){
	sprintf(buffer, "%s", receiveBuffer);
	receiveComplete = false;
	bufferPointer = 0;
}

void Leuart_SendData(char * buffer, uint8_t bufferLength)
{
	/* Timeout counter */
	uint32_t counter = 0;

	/* Send data over LEUART */
	sendLeuartData(buffer, bufferLength);

	// Start Timeout-timer if needed
	timeout = false;
	/* RTCDRV_StartTimer(xTimerForTimeout, rtcdrvTimerTypeOneshot, 10, (RTCDRV_Callback_t) &timeoutCb, NULL); */

	/* Wait for response */
	while ((counter < TIMEOUT_COUNTER) && !Leuart_ResponseAvailable()) counter++; // TODO: before: EMU_EnterEM2(true)

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_COUNTER)
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Waiting time for response reached! (Leuart_SendData)");
#endif /* DEBUGGING */

		error(53);
	}

	receiveComplete = true;
}

// Send a command string over the LEUART. Specifying a waitTime > 0 will result in a timeout when no command is received in time.
Leuart_Status_t Leuart_SendCommand(char * cb, uint8_t cbl, volatile bool * wakeUp)
{
	/* Timeout counter */
	uint32_t counter = 0;

	/* Send data over LEUART */
	sendLeuartData(cb, cbl);

	// Start Timeout-timer if needed
	timeout = false;
	//RTCDRV_StartTimer(xTimerForTimeout, rtcdrvTimerTypeOneshot, waitTime, (RTCDRV_Callback_t) &timeoutCb, NULL);

	/* Wait for response */
	while ((counter < TIMEOUT_COUNTER) && !Leuart_ResponseAvailable()) counter++; // TODO: before: EMU_EnterEM2(true) [&& !(*wakeUp)]

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_COUNTER)
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Waiting time for response reached! (Leuart_SendCommand)");
#endif /* DEBUGGING */

		error(54);
	}

	// Check for timeout
	if(timeout){ // if running is false, we got here because of the timeout.
		return (TX_TIMEOUT);
	}

	// Stop Timeout-timer when command is received
	//RTCDRV_StopTimer(xTimerForTimeout); // stop timer

	return (DATA_SENT);
}


Leuart_Status_t Leuart_WaitForResponse()
{
	/* Timeout counter */
	uint32_t counter = 0;

	// Activate DMA
	DMA_ActivateBasic(	DMA_CHANNEL_RX,
						true,
						false,
						(void *)&receiveBuffer[0],
						(void *)&RN2483_UART->RXDATA,
						0);

	// Start Timeout-timer
	timeout = false;
	//RTCDRV_StartTimer(xTimerForTimeout, rtcdrvTimerTypeOneshot, RX_TIMEOUT_DURATION, (RTCDRV_Callback_t) &timeoutCb, NULL);

	/* Wait for response */
	while ((counter < TIMEOUT_COUNTER) && !Leuart_ResponseAvailable()) counter++; // TODO: before: EMU_EnterEM2(true)

	/* Exit the function if the maximum waiting time was reached */
	if (counter == TIMEOUT_COUNTER)
	{

#ifdef DEBUGGING /* DEBUGGING */
		dbcrit("Waiting time for response reached! (Leuart_WaitForResponse)");
#endif /* DEBUGGING */

		error(55);
	}

	//RTCDRV_StopTimer(xTimerForTimeout); // stop timer

	// Check for timeout
	if(timeout){ // if running is false, we got here because of the timeout.
		return (RX_TIMEOUT);
	}

	// OK
	return (DATA_RECEIVED);
}

