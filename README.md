# Winch System Controller

A controler to manuver both the jib and the boom independently. Utilizes:
- MSP430FR5739 microcontroller to engage/disengage the pawls and rotate the drum   
- STM32F407 microcontroller which receives CAN communication and sends 2 byte commands to the MSP430
- NanoJ is used to interface with the motor controller which allows the ability to power on and off the main motor



![Winch Enclosure Diagram-Topview](https://user-images.githubusercontent.com/71032077/164404900-2a61d581-2f8b-4a76-9a5b-7d4825819505.jpg)


