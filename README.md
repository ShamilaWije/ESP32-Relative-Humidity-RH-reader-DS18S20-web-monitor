# ESP32 Relative Humidity (RH) Reader with DS18B20 and Web Monitor

This project is an ESP32-based Relative Humidity (RH) monitoring system that uses DS18B20 sensors to measure dry and wet bulb temperatures, calculate RH, and display the results. It also features web-based monitoring and control for added functionality.

## Features

- **DS18B20 Sensor Integration**: Measures dry and wet bulb temperatures for accurate relative humidity calculations.
- **Web Monitor**: Real-time monitoring and control via a WebSocket-based web interface hosted on the ESP32.
- **TM1637 Display**: Shows humidity, temperature, and system status locally using a 7-segment display.
- **Relay Control**: Automates relay switching based on RH thresholds and adjustable setpoints.
- **Wi-Fi Connectivity**: Enables remote access by connecting to a Wi-Fi network.
- **Button Inputs**: Local hardware buttons to adjust RH setpoints and toggle system states.
- **SPIFFS File System**: Hosts the web interface files directly on the ESP32.
- **ElegantOTA**: Supports over-the-air (OTA) firmware updates for easy maintenance.

## Components Used

- **ESP32**: Core microcontroller for processing and Wi-Fi connectivity.
- **DS18B20 Sensors**: For measuring dry and wet bulb temperatures.
- **TM1637**: 7-segment display for local feedback.
- **Relay Module**: To control external devices based on RH thresholds.
- **Push Buttons**: For local interaction.
