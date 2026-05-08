# Smart IoT Control Hub

ESP32-based IoT control and monitoring system with real-time telemetry, MQTT communication, relay control and ESP32-CAM video streaming.

---

# Overview

This project is a smart IoT control platform designed for remote monitoring and device management using ESP32 microcontrollers.

The system integrates:

* ESP32 main controller
* ESP32-S3-CAM for live video streaming
* MQTT communication with TLS/SSL encryption
* Remote relay control
* Android mobile application
* Real-time sensor telemetry
* WiFi failover handling using WiFiMulti
* Local TFT monitoring interface

The project focuses on embedded networking, IoT communication reliability and real-time remote control.

---

# Features

* Real-time sensor monitoring
* 3-channel relay control
* ESP32-CAM live streaming
* MQTT communication over TLS/SSL
* Android application for remote access
* Automatic WiFi reconnection using WiFiMulti
* Non-blocking firmware architecture
* Local TFT display interface via SPI
* Remote telemetry and device management
* Cloud-based communication using HiveMQ

---

# System Architecture

```text
+-------------------+
| Android App       |
| Remote Monitoring |
+---------+---------+
          |
          | MQTT / HTTP
          |
+---------v---------+
| HiveMQ Cloud      |
| MQTT Broker       |
+---------+---------+
          |
          |
+---------v---------+
| ESP32 Main Board  |
| Sensor + Relay    |
+---------+---------+
          |
          |
+---------v---------+
| ESP32-S3-CAM      |
| Video Streaming   |
+-------------------+
```

---

# Hardware Components

| Component    | Description                 |
| ------------ | --------------------------- |
| ESP32        | Main IoT controller         |
| ESP32-S3-CAM | Camera streaming module     |
| DHT22        | Temperature/Humidity sensor |
| Relay Module | Remote load control         |
| ST7735 TFT   | Local display interface     |
| Power Supply | System power source         |

---

# Communication Stack

## MQTT

The system uses MQTT protocol for:

* Sensor telemetry
* Remote relay commands
* Device status updates
* Remote monitoring

## Security

TLS/SSL encryption is implemented for secure communication with HiveMQ Cloud.

## HTTP Streaming

ESP32-CAM streaming is handled separately through HTTP tunneling for low-latency video access.

---

# Firmware Design

The firmware is designed using non-blocking programming techniques to improve responsiveness and system reliability.

## Main Tasks

* MQTT communication handling
* Sensor acquisition
* Relay control
* Camera communication
* TFT display updates
* WiFi connection management

The system avoids blocking delays to maintain stable network communication and real-time responsiveness.

---

# Android Application

A custom Android application was developed for:

* Real-time telemetry visualization
* Relay control
* Device status monitoring
* Camera stream access
* Remote IoT management

The application communicates with the ESP32 system through MQTT and HTTP services.
<img width="1080" height="2400" alt="image" src="https://github.com/user-attachments/assets/06a45422-6105-41ad-bd43-f9b09f0071c2" />


---

# Local Display Interface

The system includes a local SPI TFT display for:

* Device status visualization
* WiFi connection status
* Sensor readings
* Relay states
* System information
* <img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/18166157-7123-4b58-8516-a3b7f2007e30" />


---

# Network Reliability

WiFiMulti is implemented to improve connection stability.

Features include:

* Automatic reconnection
* Multi-network support
* Reduced downtime during network switching
* Improved long-term system stability

---

# Experimental Results

The system was tested under continuous operation conditions.

## Validation Results

* Stable MQTT communication
* Reliable relay switching
* Successful encrypted cloud communication
* Responsive Android control interface
* Stable ESP32-CAM streaming
* Improved responsiveness using non-blocking firmware
* 

---

# Challenges & Debugging

Several practical issues were encountered during development:

* ESP32 WiFi instability during long operation
* Memory limitations during camera streaming
* MQTT reconnection handling
* Blocking delay issues affecting responsiveness
* Synchronization between streaming and telemetry tasks
* Power supply noise affecting system stability

The system architecture was iteratively improved through debugging and real hardware testing.

---

# Development Tools

| Tool                     | Usage                          |
| ------------------------ | ------------------------------ |
| Arduino IDE / PlatformIO | Firmware development           |
| Android Studio           | Mobile application development |
| HiveMQ Cloud             | MQTT broker                    |
| GitHub                   | Source control                 |
| EasyEDA / Altium         | Hardware design                |

---

# Repository Structure

```text
Firmware/
├── esp32_main/
├── esp32_cam/

Android App/
├── activities/
├── mqtt_client/
├── ui/
└── streaming/

Server/

```

---

# Project Images

* ESP32 hardware setup
* Android application screenshots
* MQTT dashboard
* ESP32-CAM streaming demo
* TFT interface photos
* PCB and schematic screenshots
<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/0a662b1e-2d14-4f31-9b73-1245a19f6d9a" />

---

# Applications

Potential applications include:

* Smart home systems
* Industrial IoT monitoring
* Remote environmental sensing
* Embedded telemetry systems
* Wireless automation platforms

---

# Future Improvements

Planned improvements:

* FreeRTOS task scheduling
* OTA firmware update support
* CAN/RS485 industrial communication
* Local web dashboard
* Edge AI camera processing
* Sensor expansion support
* Power optimization

---

# Author

Nguyen Vuong Trieu

Embedded Systems | IoT Engineering | Power Electronics

GitHub: [https://github.com/Kyar4](https://github.com/Kyar4)
