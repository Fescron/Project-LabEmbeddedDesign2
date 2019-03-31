#ifdef test /* This stays here for reference */

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include <stdbool.h>
//#include "em_assert.h"

bool checker=false;


//uitleg bij de code
//Het is niet moeilijk maar ik ben moeten overstappen naar een ander concept. De weerstandswaarde kon niet laag genoeg
//worden voor een acceptabel verbruik dus ben ik veranderd van standpunt, de driftsensor zal geen interruptgeneren
//het zal ook gewoon gecheckt worden om het uur en op het moment dat de accelerometer een interrrupt genereert
// Wanneer de gecko wakker is, roep je gewoon 1 keer de functie "checkConnection" op en indien je als antwoord true hebt
// is de connectie nog steeds in orde, indien je false hebt is de kabel verbroken en is de boei op drift
// ik had eerst alles in aparte files gestoken, maar het is slechts van zo klein omvang dit project dat ik alles terug
//naar de main kopieerde voor de eenvoud





bool checkConnection(){

	//GPIO_PinOutSet(gpioPortC, 1); // alternatief voor pin mode set, weet niet wat beter of slechter is
	GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 1);

	if(!GPIO_PinInGet(gpioPortC,2)){
		return true;
	}
	else{
		return false;
	}

	GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 0);
	//GPIO_PinOutClear(gpioPortC, 1);
}








int main(void)
{
	//initialisation();
	CHIP_Init();
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIO_PinModeSet(gpioPortC, 2, gpioModeInput, 1);


  /* Infinite loop */
  while (1) {


	  checker=checkConnection();




	  EMU_EnterEM2(false);

  }
}

#endif
