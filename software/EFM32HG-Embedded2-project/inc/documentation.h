/***************************************************************************//**
 * @file documentation.h
 * @brief This file contains useful documentation about the project.
 * @version 1.2
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @mainpage Important documentation
 *
 *   In the following sections you can find some important warnings, notes and overall
 *   documentation gathered during the creation of this project. This includes, for example,
 *   info about settings done with `#define` statements or certain things found out
 *   during debugging. *Please read them!*
 *
 * ******************************************************************************
 *
 * @section Doxygen
 *
 *   This C-project has been made with [**Doxygen**](www.doxygen.org) comments above
 *   all methods to generate documentation. A combination of **Doxygen-specific tags**
 *   (preceded with an `@` symbol) and **markdown syntax** are supported to generate,
 *   for example, HTML-files.
 *
 * ******************************************************************************
 *
 * @section SETTINGS Settings using definitions in dbprint and delay functionality
 *
 *   In the file `debugging.h` **dbprint UART functionality can be enabled/disabled**
 *   with the definition `#define DEBUGGING`. If it's value is `0`, all dbprint statements
 *   are disabled throughout the source code because they're all surrounded with
 *   `#if DEBUGGING == 1 ... #endif` checks.
 *
 *   In the file `delay.h` one can **choose between SysTicks or RTC sleep functionality**
 *   for delays. This can be selected with the definition `#define SYSTICKDELAY`.
 *   If it's value is `0`, the RTC compare sleep functionality is used. if it's value
 *   is `1`, delays are generated using SysTicks.
 *
 *   In the file `delay.h` one can also **choose between the use of the ultra low-frequency
 *   RC oscillator (ULFRCO) or the low-frequency crystal oscillator (LFXO)** when being
 *   in a delay or sleeping. If the ULFRCO is selected, the MCU sleeps in EM3 and if
 *   the LFXO is selected the MCU sleeps in EM2. This can be selected with the definition
 *   `#define ULFRCO`. If it's value is `0`, the LFXO is used as the clock source. If
 *   it's value is `1` the ULFRCO is used.
 *
 *   @note Check the section @ref CLOCKS1 "Crystals and RC oscillators" for more info about this.
 *
 * ******************************************************************************
 *
 * @section Initializations
 *
 *   @warning Initializations for the methods `led(bool enabled)`, `delay(uint32_t msDelay)`
 *   and `sleep(uint32_t sSleep)` happen automatically. This is why their first call
 *   sometimes takes longer to finish than later ones.
 *
 * ******************************************************************************
 *
 * @section DEBUG Debug mode, Energy monitor and VCOM
 *
 *   @warning When in debug mode, the MCU will not go below EM1. This can cause
 *   some weird behavior. Exit debug mode and reset the MCU after flashing it for it
 *   to go in the correct sleep modes and consume the "expected" power. The Energy
 *   profiler can also be used to program the MCU but it's also necessary to reset
 *   the MCU after flashing it.
 *
 *   The Energy profiler in Simplicity Studio seems to use VCOM (on-board UART
 *   to USB converter) somehow, change to using an external UART adapter if both
 *   the energy profiler and UART debugging are necessary at the same time!
 *
 *   If the energy profiler was used and the code functionality was switched,
 *   physically re-plug the board to make sure VCOM UART starts working again!
 *
 * ******************************************************************************
 *
 * @section GPIO GPIO/DATA/BUS pins and sleep mode
 *
 *   @warning It's necessary to disable the GPIO pins of, for example, an SPI bus
 *   before going to sleep to get the *correct* sleep currents.
 *
 *   Otherwise, some unwanted currents might flow out of these pins even though
 *   the data bus isn't active. This behavior caused confusion multiple times
 *   during the development phase. It was observed that for example that a 10k
 *   pull-up resistor pulled about 320 µA of unwanted current if the corresponding
 *   MCU pins weren't disabled.
 *
 *   @warning It's also necessary to disable the GPIO pins for data pins (like for
 *   example in the case of the SPI bus) if an external sensor should be powered down.
 *
 *   Because of the low current usage of these energy-efficient sensors,
 *   it's possible they can still be parasitically powered through the data bus
 *   even though it's supply pins are powered off using GPIO pins of the MCU. This
 *   also caused confusion bugs during the development phase.
 *
 * ******************************************************************************
 *
 * @section CLOCKS1 Crystals and RC oscillators (delay.c)
 *
 *   Normally using an external oscillator/crystal uses less energy than the internal
 *   one. This external oscillator/crystal can however be omitted if the design goal
 *   is to reduce the BOM count as much as possible.
 *
 *   In the delay logic, it's possible to select the use of the ultra low-frequency
 *   RC oscillator (ULFRCO) or the low-frequency crystal oscillator (LFXO) when being
 *   in a delay or sleeping. If the ULFRCO is selected, the MCU sleeps in EM3 and if
 *   the LFXO is selected the MCU sleeps in EM2.
 *
 *   @warning After testing it was noted that the ULFRCO uses less power than the
 *   LFXO but was less precise for the wake-up times. Take not of this when selecting
 *   the necessary logic!
 *
 *   For the development of the code for the RN2483 shield from DRAMCO, it's possible
 *   they chose to use the LFXO instead of the ULFRCO in sleep because the crystal was
 *   more stable for high baudrate communication using the LEUART peripheral.
 *
 *   @note For low-power development it's advised to consistently enable peripherals and
 *   oscillators/clocks when needed and disable them afterwards. For some methods it can
 *   be useful to provide a boolean argument so that in the case of sending more bytes
 *   the clock can be disabled only after sending the latest one. Perhaps this can be
 *   used in combination with static variables in the method so they keep their value
 *   between calls and recursion can be used.
 *
 * ******************************************************************************
 *
 * @section CLOCKS2 GPIO clock (cmuClock_GPIO)
 *
 *   At one point in the development phase the clock to the GPIO peripheral was
 *   always enabled when necessary and disabled afterwards. Because the GPIO
 *   clock needs to be enabled for almost everything, even during EM2 so the MCU
 *   can react (and not only log) pin interrupts, this behavior was later scrapped.
 *
 *   `cmuClock_GPIO` is however just in case enabled in INIT methods where GPIO
 *   functionality is necessary.
 *
 *   @note Alongside this peripheral clock, `cmuClock_HFPER` is also always enabled
 *   since GPIO is a High Frequency Peripheral.
 *
 * ******************************************************************************
 *
 * @section RTCC RTCC (RTC calendar)
 *
 *   At another point in the development phase there was looked into using
 *   the RTCC (RTC calendar) to wake up the MCU every hour. This peripheral can
 *   run down to EM4H when using LFRCO, LFXO or ULFRCO. Unfortunately the Happy
 *   Gecko doesn't have this functionality so it can't be implemented in this case.
 *
 * ******************************************************************************
 *
 * @section EM Energy modes (EM1 and EM3)
 *
 *   At one point a method was developed to go in EM1 when waiting in a delay.
 *   This however didn't seem to work as intended and EM2 would also be fine.
 *   Because of this, development for this EM1 delay method was halted.
 *   EM1 is sometimes used when waiting on bits to be set.
 *
 *   When the MCU is in EM1, the clock to the CPU is disabled. All peripherals,
 *   as well as RAM and flash, are available.
 *
 *   @note In EM3, high and low frequency clocks are disabled. No oscillator (except
 *   the ULFRCO) is running. Furthermore, all unwanted oscillators are disabled
 *   in EM3. **This means that nothing needs to be manually disabled before
 *   the statement `EMU_EnterEM3(true);`.**

 *   The following modules/functions are are generally still available in EM3:
 *     @li I2C address check
 *     @li Watchdog
 *     @li Asynchronous pin interrupt
 *     @li Analog comparator (ACMP)
 *     @li Voltage comparator (VCMP)
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
 * @section DBPRINT More info about `dbprint`
 *
 *   When using `dbprint` functionality, the following settings are used:
 *     - Baudrate = 115200
 *     - 8 databits
 *     - 1 stopbit
 *     - No parity
 *
 *   When you want to debug using VCOM with interrupt functionality disabled, you
 *   can use the following initialization settings: `dbprint_INIT(USART1, 4, true, false);`
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
