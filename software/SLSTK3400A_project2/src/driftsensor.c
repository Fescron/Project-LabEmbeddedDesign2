/*
 * driftsensor.c
 *
 *  Created on: 26-mrt.-2019
 *      Author: Matthias
 */

#include <stddef.h>
#include <em_device.h>
#include <stdbool.h>
#include "em_gpio.h"

#include "../inc/driftsensor.h"

//int test=0;

/*bool checkConnection(){
	if(GPIO_PinInGet(gpioPortB,11)){
		return true;
	}
	else{
		return false;
	}

}*/


// kan gebruik worden indien we gebruik zouden maken van een interrupt, maar dit zou onnodig veel verbruiken
/*void GPIO_ODD_IRQHandler(void){


	if(GPIO_PinInGet(gpioPortB,11)){
		test++;
		for(int i=0; i<100000;i++){

		  }
		  BSP_LedsSet(1);
		  for(int i=0; i<100000;i++){

		  }
		  BSP_LedsSet(0);
	}

	GPIO_IntClear(0xAAAA);

}*/
