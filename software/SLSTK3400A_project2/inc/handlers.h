/***************************************************************************//**
 * @file handlers.h
 * @brief Interrupt handlers.
 * @version 1.2
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _HANDLERS_H_
#define _HANDLERS_H_


/* Include necessary for this header file */
#include <stdbool.h> /* "bool", "true", "false" */


/* Global variable (project-wide accessible) */
extern volatile bool triggered; /* Accelerometer triggered interrupt */


#endif /* _HANDLERS_H_ */
