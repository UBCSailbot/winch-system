# Winch System Controller

A controler to manuver both the jib and the boom independently. Utilizes:
- MSP430FR5739 microcontroller to engage/disengage the pawls and rotate the drum   
- STM32F407 microcontroller which receives CAN communication and sends 2 byte commands to the MSP430
- NanoJ is used to interface with the motor controller which allows the ability to power on and off the main motor

## Getting Started

### Prerequisite
- CCStudio (https://www.ti.com/tool/CCSTUDIO#downloads)
- Plug & Drive Studio 2 (https://en.nanotec.com/products/2533-plug-drive-studio-2)
- Putty (https://www.putty.org/)

## Programming Microcontroller
- Open CCStudios
- Right click on the Project Explorer tab and select import->CSS project
- Browse the directory with the project you want to program. You shoud be able to see the project under "Discover projects"
- Click Finished
- Do Ctrl-B with the project open. This would build the project
- Connect the black USB connection from the MSP430 to your computer
- Click the "green bug" icon on the toolbar on top to program the microcontroller
- Then press the green play button to run the program

## Winch Enclosure

Contains the electronics needed for the winch:

- **Control board PCB** has the MSP430 that is programmed with the winch-system firmware
- **UCCM** interprets CAN messages from the BBB and sends relavent commands through UART to the MSP430
- **Nanotec motor controller** is programmed with the NanoJ code and is used to rotate the drum

![Winch Enclosure Diagram-Topview](https://user-images.githubusercontent.com/71032077/164404900-2a61d581-2f8b-4a76-9a5b-7d4825819505.jpg)
