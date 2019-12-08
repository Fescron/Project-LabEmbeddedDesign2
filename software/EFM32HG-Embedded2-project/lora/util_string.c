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
 *         File: util.c
 *      Created: 2018-01-19
 *       Author: Guus Leenders - Modified by Brecht Van Eeckhoudt
 *      Version: 1.1 (1.0 -> 1.1: Added comment about malloc)
 *
 *  Description: TODO
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool StringStartsWith(char * str, char * seq){
	uint8_t i;
	for(i=0; i<strlen(seq); i++){
		if(*(str+i) != *(seq+i)){
			return (false);
		}
	}
	return (true);
}

bool HexToString(uint8_t * bin, uint8_t binsz, char **result ){
	char hex_str[] = "0123456789abcdef";
	uint8_t i;

	// ALLOCATE MEMORY
	// Hex to string/char array: one value becomes two chars (*2)
	//   +1 because of NULL termination
	// "*result =" ~ Value at memory location gets changed
	if (!(*result = (char *)malloc(binsz * 2 + 1))){
		return (false); // Returned when NULL
	}

	// Add NULL termination
	(*result)[binsz * 2] = 0; // Last memory location, "0" ~ '\0'

	// CHECK MEMORY
	if (!binsz){
		return (false); // Returned when NULL
	}

	// FILL WITH DATA
	for (i = 0; i < binsz; i++){
	  (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
	  (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
	}

	return (true);
}

char * StringToHexString(char * bin, unsigned int binsz, char **result ){
	char hex_str[] = "0123456789abcdef";
	unsigned int i;

	if (!(*result = (char *)malloc(binsz * 2 + 1))){
		return (NULL);
	}

	(*result)[binsz * 2] = 0;

	if (!binsz){
		return (NULL);
	}

	for (i = 0; i < binsz; i++){
	  (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
	  (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
	}

	return (*result);
}
