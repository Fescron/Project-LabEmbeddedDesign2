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
 *         File: leuart.h
 *      Created: 2017-12-19
 *       Author: Guus Leenders
 *
 *  Description: Header file for leuart.c
 */

#ifndef _LEUART_H_
#define _LEUART_H_

#define RECEIVE_BUFFER_SIZE 50
#define COMMAND_BUFFER_SIZE 50

typedef enum leuart_statuses{
	TX_TIMEOUT,
	RX_TIMEOUT,
	DATA_SENT,
	DATA_RECEIVED
} Leuart_Status_t;

void Leuart_Init(void);
void Leuart_Reinit(void);
void Leuart_ClearBuffers(void);
void Leuart_BreakCondition(void);
void Leuart_SendData(char * buffer, uint8_t bufferLength);
void Leuart_ReadResponse(char * buffer, uint8_t bufferLength);
Leuart_Status_t Leuart_SendCommand(char * cb, uint8_t cbl, volatile bool * wakeUp);
Leuart_Status_t Leuart_WaitForResponse();

#endif /* _LEUART_H_ */
