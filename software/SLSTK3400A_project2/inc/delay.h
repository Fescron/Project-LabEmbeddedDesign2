/***************************************************************************//**
 * @file delay.h
 * @brief Delay functionality.
 * @version 2.2
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DELAY_H_
#define _DELAY_H_


/* Includes necessary for this header file */
#include <stdint.h> /* (u)intXX_t */


/* Public definitions (for RTC compare interrupts) */
#define ULFRCOFREQ    1000
#define ULFRCOFREQ_MS 1.000
#define LFXOFREQ      32768
#define LFXOFREQ_MS   32.768


/** Public definition to select which delay to use
 *    @li `1` - Use SysTick delays.
 *    @li `0` - Use EM2/3 RTC compare delays. */
#define SYSTICKDELAY 0


/** Public definition to select the use of the crystal or the oscillator
 *    @li `0` - Use the low-frequency crystal oscillator (LFXO), EM2 sleep is used.
 *    @li `1` - Use the ultra low-frequency RC oscillator (ULFRCO), EM3 sleep is used but delays are less precise timing-wise.  */
#define ULFRCO 0


/* Public prototypes */
void delay (uint32_t msDelay);
void sleep (uint32_t sSleep);
bool RTC_checkWakeup (void);
void RTC_clearWakeup (void);


#endif /* _DELAY_H_ */
