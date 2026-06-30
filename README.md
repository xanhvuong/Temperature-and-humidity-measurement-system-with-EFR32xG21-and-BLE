# Project Overview
This project is built on the EFR32xG21 microcontroller to measure temperature and humidity using the DHT11 sensor. The measured data is displayed on an LCD and transmitted to a PC via UART. In addition, the system uses Bluetooth Low Energy (BLE) to broadcast the sensor data, allowing BLE-enabled devices (such as smartphones) to receive the information. A PC application developed in C enables users to configure system parameters and monitor the real-time temperature and humidity readings.

## Hardware Requirements
- BRD4180B Radio Board (EFR32xG21 2.4 GHz, 20 dBm)
- Wireless Starter Kit Mainboard (BRD4001A)
- DHT11 temperature and humidity sensor with connecting wires
- A BLE-enabled smartphone with the Simplicity Connect app or any equivalent Bluetooth application

<img src=https://github.com/xanhvuong/images/blob/master/EFR32xG21_BLE.png" alt="Prototype" width="854" height="480"/>
## Features
1. PC Communication via UART
The PC communicates with the EFR32xG21 through a UART interface to perform the following functions:
- Configure the temperature/humidity sampling interval.
- Configure the BLE advertising interval.
- Retrieve real-time temperature and humidity data.
- Support additional user-defined features (optional).

2. Sensor Data Acquisition and LCD Display
The EFR32xG21 periodically reads temperature and humidity values from the DHT11 sensor. The measured data is displayed on the LCD, including:
- Temperature
- Humidity
- Sensor sampling interval

3. Bluetooth Low Energy (BLE) Advertising
The EFR32xG21 broadcasts sensor data using Bluetooth Low Energy (BLE). Each advertising packet contains:
- Device name
- Temperature value
- Humidity value
This allows nearby BLE-enabled devices, such as smartphones, to receive the sensor information without requiring a direct connection.

This allows nearby BLE-enabled devices, such as smartphones, to receive the sensor information without requiring a direct connection.
