# Project-LabEmbeddedDesign2

![License](https://img.shields.io/badge/license-GNU%20GPL%20v3.0-blue.svg)
![GitHub Release Date](https://img.shields.io/github/release-date/Fescron/Project-LabEmbeddedDesign2.svg)
![GitHub release](https://img.shields.io/github/release/Fescron/Project-LabEmbeddedDesign2.svg)
![GitHub last commit](https://img.shields.io/github/last-commit/Fescron/Project-LabEmbeddedDesign2.svg)
![Target device](https://img.shields.io/badge/target%20device-EFM32HG322F64G-yellow.svg)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/Fescron/Project-LabEmbeddedDesign2.svg)

<br/>

This repository contains the **presentation**, **reports** and **code** for the project constructed for **Embedded System Design 2 - Lab**.

<br/>

The code is developed using `Simplicity Studio v4` on `Ubuntu 18.04 LTS`. The code also needs **dbprint** functionality, which needs to be added alongside the code on this repository for it to compile again. See [dbprint GIT repo](https://github.com/Fescron/dbprint) for more info regarding this.

<img src="documentation/figures/testing3.png" height="300" alt="Project"> <img src="documentation/figures/testing4.png" height="300" alt="Project">

<br/>

## 0 - Quick links

- [Reports and presentation](documentation/reports)
- [General important Doxygen documentation](https://fescron.github.io/Project-LabEmbeddedDesign2/index.html)
- [Doxygen code documentation of individual files and methods](https://fescron.github.io/Project-LabEmbeddedDesign2/files.html)
- [Schematic of custom PCB](documentation/schematics/project-embeddedSystemDesign2.pdf)
- [Schematic of DRAMCO LoRaWAN shield](documentation/schematics/dramco-LoRa-addon-schematic-v4-0.pdf)
- [Picture with PCB dimensions](https://raw.githubusercontent.com/Fescron/Project-LabEmbeddedDesign2/master/hardware/project-embeddedSystemDesign2/3d-renders/project-embeddedSystemDesign2-pcb-dimensions.png)

<br/>

- [The Things Network - Applications (Register LoRaWAN device)](https://console.thethingsnetwork.org/applications/)
- [Cayenne Dashboard](https://cayenne.mydevices.com/cayenne/dashboard/start)

<br/>

## 1 - GIT repo structure

|  Location  |  Content  |
| ---------- | --------- |
| [/docs/](https://fescron.github.io/Project-LabEmbeddedDesign2/index.html) | **[Doxygen](http://www.doxygen.org) code documentation.** |
| <br/>      |           |
| [/documentation/current measurements/](documentation/current%20measurements) | Documents containing current measurements. |
| [/documentation/current measurements/measurements-SF/](documentation/current%20measurements/measurements-SF) | Pictures depicting current profiles for sending 6 measurements at full gain with different LoRaWAN *Spreading Factors*. More information can be found in section *"3 - LoRaWAN spreading factor (RN2483)"* in [this](documentation/current%20measurements/currents-2-projectEmbeddedDesign2.pdf) file . |
| <br/>      |           |
| [/documentation/datasheets/](documentation/datasheets) | General datasheets regarding the used items. |
| [/documentation/datasheets/hardware-design/](documentation/datasheets/hardware-design) | Documents regarding hardware design for the `EFM32HG` microcontroller. |
| [/documentation/datasheets/timers-energy-rtc/](documentation/datasheets/timers-energy-rtc) | Documents regarding `timers`, the `RTC` and `energy modes` on the `EFM32HG` microcontroller. |
| <br/>      |           |
| [/documentation/figures/](documentation/figures) | Figures used in reports and the README. |
| [/documentation/reports/](documentation/reports) | **Reports and presentation.** |
| [/documentation/schematics/](documentation/schematics) | Schematics. <br/>  See [this](documentation/schematics/project-embeddedSystemDesign2.pdf) file for the schematic of the *self designed PCB* and [this](documentation/schematics/dramco-LoRa-addon-schematic-v4-0.pdf) file for the schematic of the *DRAMCO LoRaWAN shield*.
| <br/>      |           |
| [/hardware/project-embeddedSystemDesign2/](hardware/project-embeddedSystemDesign2) | [KiCad](http://kicad-pcb.org/) PCB design files. <br/> See [this](hardware/project-embeddedSystemDesign2/pdf/project-embeddedSystemDesign2.pdf) file for the *schematic* and [this](hardware/project-embeddedSystemDesign2/bom/bom-custom-gecko-v1-0.pdf) file for the *BOM*. |
| [/hardware/project-embeddedSystemDesign2/3d-renders/](hardware/project-embeddedSystemDesign2/3d-renders) | Pictures of 3D renders of the *self designed PCB*. <br/> On [this](https://raw.githubusercontent.com/Fescron/Project-LabEmbeddedDesign2/master/hardware/project-embeddedSystemDesign2/3d-renders/project-embeddedSystemDesign2-pcb-dimensions.png) picture the *dimensions* are displayed. <br/>  [This](https://raw.githubusercontent.com/Fescron/Project-LabEmbeddedDesign2/master/hardware/project-embeddedSystemDesign2/3d-renders/project-embeddedSystemDesign2.png) is a render of the *front* and [this](https://raw.githubusercontent.com/Fescron/Project-LabEmbeddedDesign2/master/hardware/project-embeddedSystemDesign2/3d-renders/project-embeddedSystemDesign2-back.png) is a render of the *back*. |
| <br/>      |           |
| [/software/EFM32HG-Embedded2-project/](software/EFM32HG-Embedded2-project) | **Code for the project.** <br/> See [this](https://fescron.github.io/Project-LabEmbeddedDesign2/index.html) page for *general important documentation* and [this](https://fescron.github.io/Project-LabEmbeddedDesign2/files.html) page for information regarding *individual files and their methods*. |

<br/>

## 2 - Code flow

![Flowchart](/documentation/figures/flowchart-full.png?raw=true "Flowchart")

**Extra notes on the flowchart:**

- **(\*1):** The following things get initialized:
    - Chip
    - GPIO wakeup (buttons & ADXL INT1)
    - ADC
    - Disable RN2483
    - Accelerometer (initialize SPI and use it to configure the accelerometer
- **(\*2):** On every external temperature measurement the VDD and DATA pin get re-configured. The oneWire protocol is achieved using *bit-banging*.
- **(\*3):** The RN2483 module gets enabled and configured before sending the data. Afterwards it gets disabled again. The **status** message is always send, measurements only if there are any recorded.
- **(\*4):** Only one set of measurements are send along with the status message.
- **(\*5):** The value of **X** depends on a configurd setting.
- **(\*6):** A status message is send along with any recorded measurements.

<br/>

- Before entering a *sleep* state **all GPIO (data) pins are disabled** so they can't consume any power.
- **Clocks and peripherals** are only enabled when necessary.
- If an **error** occures this gets **forwarded to the cloud using a LoRaWAN status message**.
- To conserve sended bytes a **custom data-format** was used (along with a *decoder*).
- Negative internal and external temperatures were successfully send and displayed. The internal temperature seems to be 2 - 3 °C off.
