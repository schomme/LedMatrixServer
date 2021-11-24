
# HTTP-Server LED-Matrix

This repository contains all files to create a simple http server for a RGB LED Matrix.
I run the code on a ESP8266 Wemos D1 mini and use a 32x64 RGB Panel.

I revieved the most informationen from this documentation:
https://www.instructables.com/RGB-LED-Matrix-With-an-ESP8266/
(thanks a lot to [@witnessmenow](https://www.github.com/witnessmenow)!)



## Setup
On the first boot the ESP will create a Wifi-AP. Connect to the AP and use the 
web interface to select your own Wifi Network and enter the password. 
The ESP will try to connect to the network. On success the display will display
the url of the LED Matrix. Default:http://LEDMATRIX/

## Reset
To disconnect the ESP from the current network and earese the credentials 
from the internal memory, press the button for at least 20sec.

## Control
Visit the web interface of the ESP.
###  Input
A simple http form to change the values of the variables 
and control the LED Matrix.
### Help
Link to this Github repository.

## HTTP Request

#### Set variables

```http
  GET /set
```

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `text` | `string` | displayed text |
| `size` | `int` | font size |
| `color_R` | `int` | red color (0-255) |
| `color_G` | `int` | gree color (0-255) |
| `color_B` | `int` | blue color (0-255) |
| `xpos` | `int` | X-Position |
| `ypos` | `int` | Y-Position |
| `brightness` | `int` | brightness (0-255) |
| `wrap` | `int` | auto line breaks. value doesn't matter. |
| `scroll` | `int` | text runs from right to left. value doesn't matter. |



## Optimizations
When the `scroll` parameter is set to 1, the text moves from right to left forever.
The X-Position should be reset to start from the right again.

## Usage
Simple python script to collect data (e.g. weather data) an send it with a http get request to the ESP in  regular intervals (cron).
