/***************************************************************************//**
 * @file delay.h
 * @brief Delay functions.
 * @version 1.3
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DELAY_H_
#define _DELAY_H_


/* Include necessary for this header file */
#include <stdint.h> /* (u)intXX_t */


/* Definitions for RTC compare interrupts */
#define LFXOFREQ 32768
#define LFXOFREQ_MS 32.768


/* Definition to select which delay to use
 * Comment this line to use EM2 delays, otherwise use SysTick delay */
//#define SYSTICKDELAY /* TODO: Move to source file? */


/* Prototypes for methods available to be used elsewhere */
void delay (uint32_t msDelay);
void sleep (uint32_t sSleep);
void initRTCcomp (void);


#endif /* _DELAY_H_ */
