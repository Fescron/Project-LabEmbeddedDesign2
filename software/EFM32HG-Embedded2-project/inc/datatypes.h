/***************************************************************************//**
 * @file datatypes.h
 * @brief Definitions of the custom data-types used.
 * @version 2.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   @li v1.0: Moved `mcu_states` enum definition from `main.c` to this file and added `MeasurementData_t` struct data type.
 *   @li v1.1: Added another `MCU_State_t` option.
 *   @li v1.2: Added another `MCU_State_t` option.
 *   @li v1.3: Changed data types in `MeasurementData_t` struct.
 *   @li v2.0: Updated version number.
 *
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DATATYPES_H_
#define _DATATYPES_H_


/* Include necessary for this header file */
#include <stdint.h> /* (u)intXX_t */


/** Enum type for the state machine */
typedef enum mcu_states
{
	INIT,
	MEASURE,
	SEND,
	SEND_STORM,
	SLEEP,
	SLEEP_HALFTIME,
	WAKEUP
} MCU_State_t;


/** Struct type to store the gathered data */
typedef struct
{
	uint8_t index;
	int32_t voltage[6];
	int32_t intTemp[6];
	int32_t extTemp[6];
} MeasurementData_t;


#endif /* _DATATYPES_H_ */
