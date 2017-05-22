# FinalDegreeProject
This repo contains the source code files needed to accomplish the contents of my final degree proyect, based on the Siemens SIMATIC IOT2040 system dedicated to the industry IoT

Since the IOT2040 is based on the Intel Galileo GEN2, its SoC contains an Arduino expansion, making it compatible with the most Arduino shields. Thus, this project relies on the usage of such Arduino capability to implement a TinkerKit! called Arduino shield, in order to make sensor/actuator installation faster and easier. 



## Milestones

NOTE: Each milestone will have its own branch on this proyect repository

### 1.Create mraa compatible TinkerKit! library
The existing [official TinkerKit! library](https://github.com/TinkerKit/TinkerKit) is based on the Arduino core libs with the idea to be working just for Arduino. However, the IOT2040 works with [Intel´s C/C++ mraa library](https://iotdk.intel.com/docs/master/mraa/) so, to be able to get access to the TinkerKit! shield I/O pins, it is a must to make an adaptation of the [TinkerKit! code for Arduino](https://github.com/TinkerKit/TinkerKit) to make it work with Intel's [mraa](https://iotdk.intel.com/docs/master/mraa/) core libraries. That said, such adaptation can be found in the [/libraries](https://github.com/bamsbamx/FinalDegreeProject/tree/master/libraries) folder of this repo.

Related files:
```
libraries/TinkerKit.cpp: TinkerKit! library source file
libraries/TinkerKit.hpp: TinkerKit! header file
```


### 2.Test UART serial communication
In this project, the IOT2040 should be able to communicate with other devices. This could be achieved via the UART capability of the Arduino expansion. The Arduino digital pins 0 and 1 are designed for serial communication, and those pins belong to the UART port 0. 

In order to achieve the serial communication between boards, a test case has been developed, where the IOT2040 reads the value of a TinkerKit! potentiometer, sends its value as a byte string to an Arduino ATMega2560 board via serial communiaction and the ATMega sends an analog signal via PWM to light a LED with a specific brightness level based on the potentiometer value. 

Related files:
```
uart_test_tx.cpp: Code necessary to make the IOT2040 read the potentiometer value and send it via serial communication
uart_test_rx.ino: Code necessary to make the Arduino ATMega2560 receive the potentiometer value via serial communication, light the LED with the received potentiometer value brightness level and debug the recevied value to the Arduino IDE Serial Monitor
```




