# LoRa Modem

LoRa Modem is a project that lets you use inexpensive LoRa radios as radio modem. It allows access via serial port and/or via Wi-Fi websocket connection. Supports configuration with AT-commands and contains simple web-interface onboard.

Firmware is based on Arduino framework and widely used RTOS capabilities for tasks management. ESP32 is used as core microcontroller and SX1278 for LoRa radio. Web-interface is based on Vue.js.

## Table of Contents
 * [Getting Started](#getting-started)
 * [Operational Principles](#operational-principles)
 * [Usage](#usage)
 * [Contribution](#contribution)
 * [Future Plans](#future-plans)
 * [License](#license)

## Getting Started

LoRa Modem uses the [PlatformIO](https://platformio.org/) development environment. In order to build and upload firmware it is necessary to have it installed:
```
pip install -U platformio
```
[Makefile](master/Makefile) allows to build and flash firmware and user interface. It is possible to build whole project and flash device by command:
```
make
```
By default expecting that device listens for `/dev/ttyUSB0` serial port. If device listens another serial please adjust [platformio.ini](include/platformio.ini).

This project was developed on [Heltec WiFi LoRa 32 (V2.1)](https://heltec.org/product/wifi-lora-32-v2/) platform. It can be adjusted for usage with another similar hardware, please refer to [config.h](include/config.h) to adjust pinout for your specific case.

## Operational Principles
Data for transmission can be entered using serial connection or websocket connection.

In case when received data can be interpreted as valid command, modem executes it and returns a response. Otherwise modem sends received data as LoRa packet.

Every modem has its ows address which is lastest 16-bits of module's Wi-Fi mac address.   

Every LoRa packet consist from source and destination addresses and payload. Payload can be up to 250 bytes size.

Modem sends periodically advertisements packets which shows that it is ready for communications. Addition to it, modem continuously listens for incoming packets. 

If incoming packet destination coinsides with modem address, then payload of message sends to the modem clients, otherwise it is ignored. If incoming packet is advertisement, then its source is added into list of known nodes. 

## Usage

This modem is configured with AT-commands. Please refer to [commands description](docs/commands.md) for full list of available commands.

Wi-Fi module can be in following states:
* OFF - disabled;
* AP - access point mode;
* STA - station mode.
Access to modem via web-interface or websocket connection available only for AP or STA modes. 

In AP mode modem creates an access point. All clients can access web-interface on http://loramodem.online URL within this access point. Requests to another URLs redirects to hostname with web-interface.

In STA mode modem should be configured with valid Wi-Fi credentials and web-interface can be accessed directly via IP address.

It is possible to transfer files using protocols XMODEM, YMODEM, ZMODEM with any existing software implementing these. Web-interface allow to send files as well.

## Contribution

Your contributions are always welcome!

Before sending a Pull Request, please make sure that you're assigned the task on a GitHub issue.

- If a relevant issue already exists, discuss on the issue and get it assigned to yourself on GitHub.
- If no relevant issue exists, open a new issue and get it assigned to yourself on GitHub.

Please proceed with a Pull Request only after you're assigned.

It is recommended to use [PlatformIO IDE](https://platformio.org/platformio-ide) for development. 

Dependencies for running tests can be installed with:
```
pip install -r dependencies.txt
```

## Future Plans

## [License](LICENSE.md)
MIT © [Denis Dechev](https://github.com/dendec)