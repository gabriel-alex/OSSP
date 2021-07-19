# Open Source Smart Plug (OSSP)

## Hardware
This project is based on ESP32 chip.

## Software
The front end use the [Milligram](https://milligram.io/) CSS framework. 

## Features
* Measure the power consumption of devices connected to the OSSP 
* Send the data to Emoncms instance.
* Create a wifi access poin to modify the settings


## Update the firmeware over the air
* Connect to the Acces Point "OSSP" with the default password("justepourtester" if not connected).
* Go to "192.168.4.1" with a web browser.
* To do a firmware update, go to "192.168.4.1/update".


## Source
[OTA example](https://randomnerdtutorials.com/esp32-ota-over-the-air-vs-code/ )
[Millgram reponsive menu](https://github.com/shuedna/Milligram-baseSite-withMenu)
[Milligram NavBar](https://gist.github.com/primaryobjects/5a86955d0419d64ae7f5c401bb704983)


