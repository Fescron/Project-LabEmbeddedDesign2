/***************************************************************************//**
 * @file cable.h
 * @brief Cable checking functionality.
 * @version 2.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _CABLE_H_
#define _CABLE_H_


/* Includes necessary for this header file */
#include <stdbool.h>   /* "bool", "true", "false" */
#include "datatypes.h" /* Definitions of the custom data-types */


/* Public prototype */
bool checkCable (MeasurementData_t data);


#endif /* _CABLE_H_ */
