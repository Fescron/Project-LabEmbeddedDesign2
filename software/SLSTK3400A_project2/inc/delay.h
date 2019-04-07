/***************************************************************************//**
 * @file delay.h
 * @brief Delay functionality.
 * @version 1.7
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DELAY_H_
#define _DELAY_H_


/** Includes necessary for this header file */
#include <stdint.h> /* (u)intXX_t */


/** Public definitions (for RTC compare interrupts) */
#define LFXOFREQ 32768
#define LFXOFREQ_MS 32.768


/** Public definition to select which delay to use */
/*   => Uncomment define to use SysTick delays
 *   => Comment define to use EM2 RTC compare delays */
//#define SYSTICKDELAY


/** Public prototypes */
void delay (uint32_t msDelay);
void sleep (uint32_t sSleep);


#endif /* _DELAY_H_ */
