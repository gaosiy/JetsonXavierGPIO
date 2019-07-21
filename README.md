# jetsonXavierGPIO
A straightforward C/C++ library to interface with the NVIDIA Jetson Xavier Development Kit GPIO pins.

Modified based on https://github.com/MengHuanLee/jetsonTX2GPIO


Based on Software by RidgeRun
https://developer.ridgerun.com/wiki/index.php/Gpio-int-test.c
 * Copyright (c) 2011, RidgeRun
 * All rights reserved.

and ideas from Derek Malloy Copyright (c) 2012
https://github.com/derekmolloy/beaglebone

exampleGPIApp.cpp describes a simple usage case using a tactile button and LED as input and output.


# Description
To add more GPIO pins, modify jetsonGPIO.h and add pin numbers you want.

* Example: 
setup and initialize GPIO417 (Pin22) as output pin on Xavier:
```
/* To initialize */
jetsonXavierGPIONumber LEDControlPin = gpio417; 
gpioExport(LEDControlPin);
gpioSetDirection(LEDControlPin,outputPin);

/* To control pin high or low  */
gpioSetValue(LEDControlPin, on);     //Pull high
gpioSetValue(LEDControlPin, off);    //Pull low
```

That's it, very easy. If you like to use C/C++ to control GPIO Pin, just include "jetsonGPIO.h" in the cpp file.

