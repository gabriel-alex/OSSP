# Open Source Smart Plug (OSSP)
This projet aims to create an Open Source Smart Power Plug in an instructional perspective. Smart Power Plugs are quite common on online marketplace. Most of them a based on [Tuya Smart](https://www.tuya.com/) white labeled product. However, there is two main challenges related to there products: transparency about the data processing and evolutivity. The ambition of this project is to create a device that allow a large set of experimentation on elecricity monitoring with highest level of transparency related to measurement and data processing. 

## Origin
Several limitations has been identified from the actual open source projects related to energy monitoring: 
- Open source projects identified can either pilot the electrical device (with a relay) or measure the power consumption. 
- The most important project related to energy monitoring i.e. [OpenEnergyMonitor](https://learn.openenergymonitor.org) provides various instructional content to build its own monitoring devices based on arduino while comercial solution are "smart devices". 

## Hardware
This project is based on ESP32 chip.
Various experimentation have been done relative to the components used to measure or control the power. 

## Software
The front end use the [Milligram](https://milligram.io/) CSS framework. 
It is also based on library such as [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA) and [Emonlib](https://github.com/openenergymonitor/EmonLib).

## Features
* Measure the power consumption of the device connected to the OSSP 
* Send the data to an instance of Emoncms
* Create a wifi access point to modify the settings if there is issue to connect to a network
* Control the relay to activate/ desactivate the load of the electric device connected to the OSSP

Further details about the progress of this project is avaialbe [here](./doc/TODO.md).


## Update the firmeware over the air
* Connect to the Acces Point "OSSP" with the default password("justepourtester" if not connected).
* Go to "192.168.4.1" with a web browser.
* To do a firmware update, go to "192.168.4.1/update".


## Source
[OTA example](https://randomnerdtutorials.com/esp32-ota-over-the-air-vs-code/ )  
[Millgram reponsive menu](https://github.com/shuedna/Milligram-baseSite-withMenu)  
[Milligram NavBar](https://gist.github.com/primaryobjects/5a86955d0419d64ae7f5c401bb704983)

[Arduino energy monitor](https://learn.openenergymonitor.org/electricity-monitoring/ctac/how-to-build-an-arduino-energy-monitor)  
[EmonESP](https://github.com/openenergymonitor/EmonESP)

[CSS framework compariosn](https://codeburst.io/evaluating-css-frameworks-bulma-vs-foundation-vs-milligram-vs-pure-vs-semantic-vs-uikit-503883bd25a3)
[Alert banner](https://codepen.io/rlemon/pen/vmIlC)
[environment varialbe](https://community.platformio.org/t/pass-env-variables-at-build-time/4622/4)