# Early Warning Fire Alert System

The file lora_fire_node_master.ino is to be flashed onto a Heltec esp32 lora board. A LM335 temperature is connected to the pin 13 of the board and the sensor reads the pin value for 30s every minute, checks for incoming TX lora signals, and if the sensor reading is above a configured threshold, or if there is an incoming fire alert, the board starts broadcasting a fire message itself. 

The ewaff_Server package is compiled and flashed onto a DE10 Nano Intel FPGA. It will host a Wifi Access point and a web server which will let us know if the board sees any incoming fire alert signal from any of the nodes in its periphery. The web pages are displayed at 192.168.4.1. Any fire data from previous alerts is hosted at /fire_data.html. 
