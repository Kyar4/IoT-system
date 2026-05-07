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

---

# Local Display Interface

The system includes a local SPI TFT display for:

* Device status visualization
* WiFi connection status
* Sensor readings
* Relay states
* System information

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

Recommended repository contents:

* ESP32 hardware setup
* Android application screenshots
* MQTT dashboard
* ESP32-CAM streaming demo
* TFT interface photos
* PCB and schematic screenshots

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
