# Cypress Emulated EEPROM Middleware Library 2.0

### What's Included?
Please refer to the [README.md](./README.md) and the [API Reference Guide](https://cypresssemiconductorco.github.io/emeeprom/em_eeprom_api_reference_manual/html/index.html) for a complete description of the Emulated EEPROM Middleware.
The revision history of the Emulated EEPROM Middleware is also available on the [API Reference Guide Changelog](https://cypresssemiconductorco.github.io/emeeprom/em_eeprom_api_reference_manual/html/index.html#section_em_eeprom_changelog).
New in this release:
* Updated major and minor version defines for consistency with other libraries
* Updated documentation for user experience improvement
* Added migration guide from PSoC Creator Em EEPROM component
* Added mechanism to restore corrupted redundant copy from the main data copy

### Defect Fixes
* Fixed MISRA Violation
* Fixed defect of the Cy_Em_EEPROM_Read() function when Emulated EEPROM data corruption in some cases caused infinite loop.
* Fixed defect of the Cy_Em_EEPROM_Read() function when the function returns incorrect data after restoring data from redundant copy.


### Supported Software and Tools
This version of the Emulated EEPROM Middleware was validated for the compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| ModusToolbox Software Environment                       | 2.0     |
| PSoC6 Peripheral Driver Library (PDL)                   | 1.2.0   |
| GCC Compiler                                            | 7.2.1   |
| IAR Compiler                                            | 8.32    |
| ARM Compiler 6                                          | 6.11    |
| MBED OS                                                 | 5.13.1  |
| FreeRTOS                                                | 10.0.1  |

### More information
For more information, refer to the following documents:
* [Emulated EEPROM Middleware README.md](./README.md)
* [Emulated EEPROM Middleware API Reference Guide](https://cypresssemiconductorco.github.io/emeeprom/em_eeprom_api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [PDL API Reference](https://github.com/cypresssemiconductorco.github.io/psoc6pdl/pdl_api_reference_manual/html/index.html)
* [AN219434 Importing PSoC Creator Code into an IDE for a PSoC 6 Project](https://www.cypress.com/an219434)
* [AN210781 Getting Started with PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity](http://www.cypress.com/an210781)
* [PSoC 6 Technical Reference Manual](https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference)
* [PSoC 63 with BLE Datasheet Programmable System-on-Chip datasheet](http://www.cypress.com/ds218787)
* [Cypress Semiconductor](http://www.cypress.com)
  
---
© Cypress Semiconductor Corporation, 2019.
