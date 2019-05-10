# Project-LabEmbeddedDesign2

![License](https://img.shields.io/badge/license-GNU%20GPL%20v3.0-blue.svg)
![GitHub Release Date](https://img.shields.io/github/release-date/Fescron/Project-LabEmbeddedDesign2.svg)
![GitHub release](https://img.shields.io/github/release/Fescron/Project-LabEmbeddedDesign2.svg)
![GitHub last commit](https://img.shields.io/github/last-commit/Fescron/Project-LabEmbeddedDesign2.svg)
![Target device](https://img.shields.io/badge/target%20device-EFM32HG322F64G-yellow.svg)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/Fescron/Project-LabEmbeddedDesign2.svg)

<br/>

The code is developed using `Simplicity Studio v4` on `Ubuntu 18.04 LTS`. The code also needs **dbprint** functionality, which needs to be added alongside the code on this repository for it to compile again. See [dbprint GIT repo](https://github.com/Fescron/dbprint) for more info regarding this.

<br/>

## Software documentation
The following links go to code documentation with [Doxygen](http://www.doxygen.org).
- [General important documentation](https://fescron.github.io/Project-LabEmbeddedDesign2/index.html)
- [Individual files and their methods](https://fescron.github.io/Project-LabEmbeddedDesign2/files.html)

<br/>

## Code flow

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

<br/>

## LoRaWAN links
- [The Things Network - Applications (Register LoRaWAN device)](https://console.thethingsnetwork.org/applications/)
- [Cayenne Dashboard](https://cayenne.mydevices.com/cayenne/dashboard/start)

<br/>

## Important hardware files
- [Top Copper layer (1:1)](https://github.com/Fescron/Project-LabEmbeddedDesign2/blob/master/hardware/project-embeddedSystemDesign2/pdf/project-embeddedSystemDesign2-F_Cu.pdf)
- [Bottom Copper layer (1:1, mirrored)](https://github.com/Fescron/Project-LabEmbeddedDesign2/blob/master/hardware/project-embeddedSystemDesign2/pdf/project-embeddedSystemDesign2-B_Cu-mirrored.pdf)

<br/>

- [Schematic](https://github.com/Fescron/Project-LabEmbeddedDesign2/blob/master/hardware/project-embeddedSystemDesign2/pdf/project-embeddedSystemDesign2.pdf)
- [BOM](https://github.com/Fescron/Project-LabEmbeddedDesign2/blob/master/hardware/project-embeddedSystemDesign2/bom/bom-custom-gecko-v1-0.pdf)

<br/>

- [3D-render front](https://raw.githubusercontent.com/Fescron/Project-LabEmbeddedDesign2/master/hardware/project-embeddedSystemDesign2/3d-renders/project-embeddedSystemDesign2.png)
- [3D-render back](https://raw.githubusercontent.com/Fescron/Project-LabEmbeddedDesign2/master/hardware/project-embeddedSystemDesign2/3d-renders/project-embeddedSystemDesign2-back.png)

<br/>

- [PCB with dimensions](https://raw.githubusercontent.com/Fescron/Project-LabEmbeddedDesign2/master/hardware/project-embeddedSystemDesign2/3d-renders/project-embeddedSystemDesign2-pcb-dimensions.png)

<br/>
