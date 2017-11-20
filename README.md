# Remote Control Led on BBG with LED char driver 

This assignment is to create a char driver for on board device led and an led  server driver enabling remote control of led from client 

### Prerequisites

* Clone the repository 
* move myled.c in the newly created char driver folder in /usr/src/linux headers/drivers/... 
* Add a makefile in the folder for compiling kernel module
* The driver is for blinking led on 53 port.
* Do a sudo insmod myled.ko for installing the module
* Open /sys/ecen5013/led53/ folder
* You can get/set the blink period,blink dutycycle,and led state.
* Look into modinfo of the module for the attribute limitations.
* Now from user space of BBG run led_driver which is server running on port 12345
* Run client.c on laptop as a client 
* select  among the list of options and now you can remotley control leds

## Authors
Mounika Reddy Edula

