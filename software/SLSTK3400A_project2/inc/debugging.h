/***************************************************************************//**
 * @file debugging.h
 * @brief Enable or disable printing to UART.
 * @details
 *   This header file is called in every other file where there are UART
 *   debugging statements. Commenting the line in this file can remove all
 *   UART functionality in the whole project.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DEBUGGING_H_
#define _DEBUGGING_H_


/** Comment the line below to remove all USART debugging stuff */
#define DEBUGGING


#ifdef DEBUGGING /* DEBUGGING */
#include "dbprint.h"
#endif /* DEBUGGING */


#endif /* _DEBUGGING_H_ */
