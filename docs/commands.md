# AT Commands
Every AT command should be terminated by line feed (`\n`)  character and send at once. It is possible to use any serial port communications program for this purpose, for example: [cutecom](https://help.ubuntu.com/community/Cutecom). 

| Command | Description | Result |
|---|---|---|
| `AT` | Modem availability check | `OK` |
| `ATZ` | Reset all settings to default values | `OK` |
| `AT+RST` | Rebooting the radio and CPU |  |
| [`AT+CONF`](#atconf) | Display all radio settings | `1234,434.0,500.0,9,7,10,0` |
| `AT+ADDR` | Getting address | `1234` |
| `AT+FREQ` | Getting frequency in MHz | `434.0` |
| `AT+FREQ=<x>` | Setting frequency in MHz. Valid values: 410.0 – 525.0 | `OK/ERROR` |
| `AT+BW` | Getting channel width in kHz | `500.0` |
| `AT+BW=<x>` | Setting channel width in kHz. Valid values: 7.8, 10.4, 15.6, 20.8, 31.2, 41.7, 62.5, 125.0, 250.0, 500.0 | `OK/ERROR` |
| `AT+SF` | Getting spreading factor | `9` |
| `AT+SF=<x>` | Setting spreading factor. Valid values: 6, 7, 8, 9, 10, 11, 12 | `OK/ERROR` |
| `AT+RATE` | Getting code rate | `7` |
| `AT+RATE=<x>` | Setting code rate. Valid values: 5, 6, 7, 8 | `OK/ERROR` |
| `AT+POW` | Getting transmitter power in dBm | `10` |
| `AT+POW=<x>` | Setting transmitter power in dBm. Valid values: 2-17, 20 | `OK/ERROR` |
| [`AT+ADV`](#atadv) | Getting interval between advertising packets in ms | `10000` |
| `AT+ADV=<x>` | Setting interval between advertising packets in ms. Valid values: 0 - 60000 | `OK/ERROR` |
| [`AT+MODE`](#atmode) | Getting current mode of operation | `1` |
| `AT+MODE=<x>` | Setting operating mode. Valid values: 0 - 27 | `OK/ERROR` |
| `AT+SCAN` | Getting a list of visible devices. Each element of the list contains: address, signal level, time until the last detection. Time and period in milliseconds. | `4321,-46.0,2445` |
| `AT+STAT` | Getting network traffic statistics: number of bytes sent and received | `0,0` |
| [`AT+WIFI`](#atwifi) | Getting Wi-Fi settings | `2,1,ssidname,*` |
| `AT+WIFI=<x>` | Setting Wi-Fi settings | `OK/ERROR` |
| `AT+IP` | Getting local IP address | `192.168.0.4`|

It is recommended to be familiar with LoRa radio parameters in order to choose them properly according to communication conditions. 

Some commands require more detailed explanation: 

### `AT+CONF`

Displays all radio settings in following order: address, frequency, channel width, spreading factor, code rate, power, gain. 
- Address is hexadecimal integer equals to last 16 bits of module's Wi-Fi MAC address. 
- Frequency is a float value equals to base frequency in MHz. 
- Channel width is a float value equals to bandwiths in kHz.
- Spreading factor is an integer number. Modulation parameter which reflects chirp duration. Large spreading factors offer low data rates but gives higher communication distance.
- Code rate is an integer number. Reflects amount of redundancy added to payload. If there are too many interference in the channel, then it’s recommended to increase the value of CR. However, the rise in CR value will also increase the duration for the transmission.
- Power is integer number. Shows transmitter output power in dBm.
- Gain is a binary value. Shows if automatic gain control enabled.

**Important note:** frequency, bandwidth, code rate and spreading factor have to be the same for both communication sides.

### `AT+ADV`

Is is not recommended to too low use advertisement period, because advertising packets will create a lot of traffic in air. This can decrease quality of communication. If period value equals to `0` no any advertizing packets send. With this setting other nodes are unable to discover yours.

### `AT+MODE`

Mode combines radio settings related to modulation and data encoding in following way: lower mode provides higher data rate and lower communication distance. Find below list of all possible modes:

| Mode | SF | CR | Max bitrate, bit/s |
|---|---|---|---|
| 0 | 6 | 5 | 32000 |
| 1 | 6 | 6 | 28008 |
| 2 | 6 | 7 | 24514 |
| 3 | 6 | 8 | 21456 |
| 4 | 7 | 5 | 18426 |
| 5 | 7 | 6 | 16128 |
| 6 | 7 | 7 | 14116 |
| 7 | 7 | 8 | 12355 |
| 8 | 8 | 5 | 10610 |
| 9 | 8 | 6 | 9287 |
| 10 | 8 | 7 | 8128 |
| 11 | 8 | 8 | 7114 |
| 12 | 9 | 5 | 6110 |
| 13 | 9 | 6 | 5348 |
| 14 | 9 | 7 | 4680 |
| 15 | 9 | 8 | 4096 |
| 16 | 10 | 5 | 3518 |
| 17 | 10 | 6 | 3080 |
| 18 | 10 | 7 | 2695 |
| 19 | 10 | 8 | 2359 |
| 20 | 11 | 5 | 2026 |
| 21 | 11 | 6 | 1774 |
| 22 | 11 | 7 | 1552 |
| 23 | 11 | 8 | 1358 |
| 24 | 12 | 5 | 1167 |
| 25 | 12 | 6 | 1022 |
| 26 | 12 | 7 | 894 |
| 27 | 12 | 8 | 782 |

Please note, max bitrate column is theoretical estimation for bandwith 500 kHz. Data rate also depends on payload and preamble size and can be lower in real applications. Data rate depends proportionally to channel bandwidth.

### `AT+WIFI`

Displays all Wi-Fi settings in following order: Wi-Fi mode, channel, ssid, password.

- Wi-Fi mode is an integer number which has following meaning:
    - 0 - OFF.
    - 1 - AP. Access point mode: stations can connect to the ESP32.
    - 2 - STA. Station mode: the ESP32 connects to an access point.
- Channel is an integer number in range 1-13.
- Ssid is a string with name for the access point(maximum 63 characters).
- Password is a string with minimum of 8 characters. Set to empty if you want the access point to be open or connect to open access point. 