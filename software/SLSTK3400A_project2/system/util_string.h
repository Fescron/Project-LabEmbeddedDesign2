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
 *       Author: Guus Leenders
 *      Version: 1.0
 *
 *  Description: TODO
 */

#ifndef INC_UTIL_H_
#define INC_UTIL_H_

#include <stdint.h>
#include <stdbool.h>

bool StringStartsWith(char * str, char * seq);
bool HexToString(uint8_t * bin, uint8_t binsz, char **result );
char * StringToHexString(char * bin, unsigned int binsz, char **result );

#endif /* INC_UTIL_H_ */
