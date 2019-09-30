/***************************************************************************//**
 * @file dbprint_documentation.h
 * @brief This file contains useful documentation closely related to dbprint.
 * @version 6.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @mainpage DeBugPrint
 *
 *   DeBugPrint is a homebrew minimal low-level println/printf replacement.
 *   It can be used to to print text/values to uart without a lot of external
 *   libraries. The end goal was to use no external libraries (with methods
 *   like `itoa`) apart from the ones specific to the microcontroller.
 *
 * ******************************************************************************
 *
 * @section Doxygen
 *
 *   These files have been made with [**Doxygen**](www.doxygen.org) comments above
 *   all methods to generate documentation. A combination of **Doxygen-specific tags**
 *   (preceded with an `@` symbol) and **markdown syntax** are supported to generate,
 *   for example, HTML-files.
 *
 * ******************************************************************************
 *
 * @section ENABLE Enable/disable dbprint using definition in `debug_dbprint.h`
 *
 *   In the file `debug_dbprint.h` **dbprint UART functionality can be enabled/disabled**
 *   with the definition `#define DEBUG_DBPRINT`. If it's value is `0`, all dbprint
 *   functionality is disabled.
 *
 *   @note
 *     This means that the **only header file to include in your projects** for
 *     dbprint to work is@n
 *     `#include debug_dbprint.h`
 *
 *   @note
 *     If you also want to use this definition to enable/disable dbprint statements
 *     in your code, please use the following convention:@n
 *     `#if DEBUG_DBPRINT == 1 // DEBUG_DBPRINT `@n
 *     `<your source code dbprint statements go here>`@n
 *     `#endif // DEBUG_DBPRINT`
 *
 * ******************************************************************************
 *
 * @section DBPRINT More info about dbprint and VCOM
 *
 *   VCOM is an on-board (SLSTK3400A) UART to USB converter alongside the Segger
 *   J-Link debugger, connected with microcontroller pins `PA0` (RX) and `PF2` (TX).
 *   This converter can then be used with Putty or another serial port program.
 *
 *   @note
 *     When you want to debug using VCOM with interrupt functionality disabled, you
 *     can use the following initialization settings:@n
 *     `dbprint_INIT(USART1, 4, true, false);`
 *
 *   @note
 *     When using `dbprint` functionality, the following settings are used:@n
 *       - Baudrate = 115200
 *       - 8 databits
 *       - 1 stopbit
 *       - No parity
 *
 * ******************************************************************************
 *
 * @section ENERGY Energy profiler and dbprint
 *
 *   The Energy profiler in Simplicity Studio seems to use VCOM  somehow, change
 *   to using an external UART adapter if both the energy profiler and UART
 *   debugging are necessary at the same time!
 *
 *   If the energy profiler was used and the code functionality was switched,
 *   physically re-plug the board to make sure VCOM UART starts working again!
 *
 * ******************************************************************************
 *
 * @section UART Alternate UART Functionality Pinout
 *
 *   |  Location  |  `#0`  |  `#1`  |  `#2`  |  `#3`  |  `#4`  |  `#5`  |  `#6`  |
 *   | ---------- | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
 *   |  `US0_RX`  | `PE11` |        | `PC10` | `PE12` | `PB08` | `PC01` | `PC01` |
 *   |  `US0_TX`  | `PE10` |        |        | `PE13` | `PB07` | `PC00` | `PC00` |
 *   |            |        |        |        |        |        |        |        |
 *   |  `US1_RX`  | `PC01` |        | `PD06` | `PD06` | `PA00` | `PC02` |        |
 *   |  `US1_TX`  | `PC00` |        | `PD07` | `PD07` | `PF02` | `PC01` |        |
 *
 *   VCOM:
 *     - USART1 #4 (USART0 can't be used)
 *     - RX - `PA0`
 *     - TX - `PF2`
 *
 * ******************************************************************************
 *
 * @section Keywords
 *
 *   @subsection Volatile
 *
 *   The `volatile` type indicates to the compiler that the data is not normal memory,
 *   and could change at unexpected times. Hardware registers are often volatile,
 *   and so are variables which get changed in interrupts.
 *
 *   @subsection Extern
 *
 *   Declare the global variables in headers (and use the `extern` keyword there)
 *   and actually define them in the appropriate source file.
 *
 *   @subsection Static
 *
 *   - **Static variable inside a function:** The variable keeps its value between invocations.
 *   - **Static global variable or function:** The variable or function is only "seen" in the file it's declared in.
 *
 * ******************************************************************************
 *
 * @section DATA Bits, bytes, nibbles and unsigned/signed integer value ranges
 *
 *   - 1 nibble = 4 bits (`0b1111`      = `0xF` )
 *   - 1 byte   = 8 bits (`0b1111 1111` = `0xFF`)
 *
 *  | Type       | Alias            | Size    | Minimum value  | Maximum value                 |
 *  | ---------- | ---------------- | ------- | -------------- | ----------------------------- |
 *  | `uint8_t`  | `unsigned char`  | 1 byte  | 0              | 255 (`0xFF`)                  |
 *  | `uint16_t` | `unsigned short` | 2 bytes | 0              | 65 535 (`0xFFFF`)             |
 *  | `uint32_t` | `unsigned int`   | 4 bytes | 0              | 4 294 967 295 (`0xFFFF FFFF`) |
 *  |            |                  |         |                |                               |
 *  | `int8_t`   | `signed char`    | 1 byte  | -128           | 127                           |
 *  | `int16_t`  | `signed short`   | 2 bytes | -32 768        | 32 767                        |
 *  | `int32_t`  | `signed int`     | 4 bytes | -2 147 483 648 | 2 147 483 647                 |
 *
 ******************************************************************************/
