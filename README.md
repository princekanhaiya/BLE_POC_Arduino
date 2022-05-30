# Mobile As a Key firmware <img align="center" src="https://emoji-uc.akamaized.net/orig/fc/bd01dcfc40ce4d3460efa89593adee.png" length="40" width="40" title="BLE icon">

[![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/princekanhaiya/BLE_POC_Arduino)](https://github.com/princekanhaiya) [![license](https://img.shields.io/badge/license-MIT-green)](https://github.com/princekanhaiya) [![total lines](https://img.shields.io/tokei/lines/github/princekanhaiya/BLE_POC_Arduino)](https://github.com/princekanhaiya) [![languages](https://img.shields.io/badge/language-java-blue)](https://github.com/princekanhaiya) [![languages](https://img.shields.io/github/languages/count/princekanhaiya/BLE_POC_Arduino)](https://github.com/princekanhaiya) [![forks](https://img.shields.io/github/forks/princekanhaiya/BLE_APP?style=social)](https://github.com/princekanhaiya) [![forks](https://img.shields.io/badge/linkedin-Prince%20Kanhaiya-blue)](https://in.linkedin.com/in/prince-kanhaiya)
![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)
![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=flat&logo=Arduino&logoColor=white)

## System Architecture
- This project enable the user to perform passive/active locking & unlocking of car, Controlling AC/Wiper/Window/Sunroof/Tailgate of car, Searching of Car and Starting the vehicle remotly using devices mobile phones, wearables ie wathces.
- The project also enable the user to share the Key to their friend for a defined duration of time and with other feature ie limited speed/geography.

<img align="center" src="https://embed.creately.com/AjpIv1ZJNKm?type=svg">

## Key Sharing Architecture
<img align="center" src="https://embed.creately.com/O9LxKUN1HTF?type=svg">

## Introduction of BLE Module ##
The BLE based smart phone access POC will assist the user to control their vehicle using android application either remotely or passively. This module is divided into two parts
- Software Application
- Hardware Module

## Application Feature
-	Signing and Signup screen
-	Vehicle linking process
-	Enable/Disable BLE access
-	BLE Bonding
-	Walk away lock 
-	Approach unlocks
-	Switch option to enable/disable passive locking/unlocking
-	AC Start
-	Remote Vehicle Start with timer
-	Calibration of module
-  Virtual key sharing

## Software Application Screenshots ##
<img align="center" src="https://s3.amazonaws.com/user-media.venngage.com/16363423-fdc4eefaa6c1d366adfcbe1ce94c029c.png">

## Application Flow chart ##
<img align="center" src="https://embed.creately.com/kfTYcyO6vhZ?type=svg">

- In the main dashboard there is a slide button which when enabled means passive locking is enabled.
- In passive locking, when a user keeping their phone in pocket approach to the car, central gate automatically unlocked and when user walk away from the car, it gets automatically locked.
- When slide button is disabled means passive locking is disabled, and every function works manually as intended.

## FIle Structure
* [BLE Application]()
    * [build](/build)
    * [gradle](/gradle)
    * [main](/app/src/main)
        * [AndroidManifest.xml](/app/src/main/AndroidManifest.xml)
        * [app](/app/src/main/java/com/mnm/ble_app)
            * [SplashScreen.java](/app/src/main/java/com/mnm/ble_app/SplashScreen.java) 
            * [login](/app/src/main/java/com/mnm/ble_app/login)
                * [LoginActivity.java](/app/src/main/java/com/mnm/ble_app/login/LoginActivity.java)
                * [VerifyPhoneActivity.java](/app/src/main/java/com/mnm/ble_app/login/VerifyPhoneActivity.java)
                * [CountryData.java](/app/src/main/java/com/mnm/ble_app/login/CountryData.java)
            * [MainActivity.java](/app/src/main/java/com/mnm/ble_app/MainActivity.java)
            * [CalibrationActivity.java](/app/src/main/java/com/mnm/ble_app/CalibrationActivity.java)
            * [bluetooth](/app/src/main/java/com/mnm/ble_app/bluetooth)
                * [BluetoothUuid.java](/app/src/main/java/com/mnm/ble_app/bluetooth/BluetoothUuid.java)
                * [ByteQueue.java](/app/src/main/java/com/mnm/ble_app/bluetooth/ByteQueue.java)
                * [CharSelectionAdapter.java](/app/src/main/java/com/mnm/ble_app/bluetooth/CharSelectionAdapter.java)
                * [CharSelectionDialog.java](/app/src/main/java/com/mnm/ble_app/bluetooth/CharSelectionDialog.java)
                * [ChatActivity.java](/app/src/main/java/com/mnm/ble_app/bluetooth/ChatActivity.java)
                * [ChatAdapter.java](/app/src/main/java/com/mnm/ble_app/bluetooth/ChatAdapter.java)
                * [ChatModel.java](/app/src/main/java/com/mnm/ble_app/bluetooth/ChatModel.java)
                * [randomtest.java](/app/src/main/java/com/mnm/ble_app/bluetooth/randomtest.java)
                * [SampleGattAttributes.java](/app/src/main/java/com/mnm/ble_app/bluetooth/SampleGattAttributes.java)
                * [bluetooth/ScanRecord.java](/app/src/main/java/com/mnm/ble_app/bluetooth/ScanRecord.java)
                * [Utils.java](/app/src/main/java/com/mnm/ble_app/bluetooth/Utils.java)
            * [scan](/app/src/main/java/com/mnm/ble_app/scan/)
                * [ScanActivity.java](/app/src/main/java/com/mnm/ble_app/scan/ScanActivity.java)
                * [ScanAdapter.java](/app/src/main/java/com/mnm/ble_app/scan/ScanAdapter.java)
                * [ScanModel.java](/app/src/main/java/com/mnm/ble_app/scan/ScanModel.java)

## Hardware Module ##
Hardware modules consist of followings hardware.
- Microcontroller- Atmega328P-PU
- BLE Module -HC08 (CC2541 – TI Chip)
- CAN Interface (MCP2551 High-speed CAN Transceiver)
- Crystal LCD (Only for prototype)
- I2C Module (Only for prototype)
- Indicators LED (Only for prototype)
- Power regulator
- Relay for ignition and crank
- Transistors array for switching
- NFC Reader

### Microcontroller:
The high-performance Microchip Pico Power® 8-bit AVR® RISC-based microcontroller combines 32 KB ISP Flash memory with read-while-write capabilities, 1024B EEPROM, 2 KB SRAM, 23 general purpose I/O lines, 32 general purpose working registers, three flexible timer/counters with compare modes, internal and external interrupts, serial programmable USART, a byte-oriented Two-Wire serial interface, SPI serial port, a 6-channel 10-bit A/D converter (8-channels in TQFP and QFN/MLF packages), programmable watchdog timer with internal oscillator, and five software selectable power saving modes. The device operates between 1.8-5.5 volts.
By executing powerful instructions in a single clock cycle, the device achieves throughputs approaching one MIPS per MHz, balancing power consumption and processing speed.

### BLE Chip:
The CC2541-Q1 is a power-optimized true Wireless MCU solution for both Bluetooth low energy and proprietary 2.4-GHz applications. This device enables the building of robust network nodes with low total bill-of-material costs. The CC2541-Q1 combines the excellent performance of a leading RF transceiver with an industry-standard enhanced 8051 MCU, in-system programmable flash memory, 8KB of RAM, and many other powerful supporting features and peripherals. The CC2541-Q1 is highly suited for systems in which ultralow power consumption is required, which is specified by various operating modes. Short transition times between operating modes further enable low power consumption.

### CAN Interface (MCP2551 High-speed CAN Transceiver):
The MCP2551 is a high-speed CAN transceiver, fault-tolerant device that serves as the interface between a CAN protocol controller and the physical bus. The MCP2551 provides differential transmit and receive capability for the CAN protocol controller and is fully compatible with the ISO-11898 standard, including 24V requirements. It will operate at speeds of up to 1 Mb/s.

### Crystal LCD and Indicators LED:
Crystal LCD with Philips I2C module and LED indicators are optional hardware which is only used for indication purposes while development of prototype.
NFC Reader:
There are two module named driver door NFC reader and Engine side NFC reader Module.


## Some Important points regarding Bluetooth Low Energy ##
- Despite formulas suggested by signal theory, the most accurate predictor of distance based on signal strength (RSSI) can be obtained by doing a power regression against a known table of distance/RSSI values for a specific device. This uses the formula d=A*(r/t)^B+C, where d is the distance in meters, r is the RSSI measured by the device and t is the reference RSSI at 1 meter. A, B, and C are constants.
- RSSI Problems To achieve a high accuracy for localization, reliable approaches to measure RSSI are required. Therefore, it is fundamental to understand the behaviours of the measured RSSI values. Radio signal waves from Bluetooth RSSI is influenced by interference from the environment, which is mainly caused by reflections, i.e., when the signal wave hits an object. These reflections can lead to multiple paths or fading signals that cause incorrect RSSI measurements.
- The peak current is more than 30mA (when RF power is 4dBm).

<img align="center" src="https://embed.creately.com/iHhjzBistAi?type=svg">

- When done trilateration with 3-BLE module there is ±5m error also distance vary from time to time depending upon several parameters i.e., amount of RF reflections, Environment, receiving sensor placement and sensor manufacturers. Hence from BLE we can’t find accurate distance in automotive like environment.


