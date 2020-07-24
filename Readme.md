---
title: Analog Voltage MQTT Client
author: Alfons Schuck
date: 25.07.2020
---

# Analog Voltage MQTT Client
## Description
This Sketch for an Arduino Uno in combination with a MCP4725 DAC can measure two voltages between 0V and 5V and set one output between 0V and 5V. This setup is therefore able to control a Delta Electronica Laboratory Power Supply through the 15 Pin D-Sub connector.

Additionally, an Ethernet Shield is used to send and receive measurement- and control parameters via MQTT.
