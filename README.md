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


### 3.TCP/IP communication inside a LAN
This time the IOT2040 Ethernet capability is going to be tested, in order for it to be able to connect and communicate with other devices connected into the same LAN. Again, an Arduino board will be used for the experiment (obviously with an Arduino Ethernet shield attached to it). The chosen communication protocol is TCP/IP, because it is probably the most used communication protocol between Internet connected devices and because both the IOT2040 and Arduino are compatible with it.

In order to create more "formal" communication, an abstract command interface has been introduced keeping in mind the target of this project, which is to make possible to manipulate electronic elements such as sensors/actuators via Internet of Things. Hence, the interface consists of two pair of commands: analog read/write and digital read/write. With them, its possible to cover the most part of available elements in an industry:
```
"ar <analogPinNumber>\n": Analog read command. The client will read the analog value of the <pinNumber> assigned pin, and reply back with the read value concatenated to the actual command (Example: "ar 1\n" makes the client read the pin 6 analog value and reply "ar 1 347\n") NOTE: The ADC value range may depend on the client so, in case of using an Arduino board, the actual range will be 0-1023

"aw <analogPinNumber> <dacValue>\n": Analog write command. Sets the <pinNumber> assigned analog pin the analog <dacValue> The client will reply with the same command if the execution was successful. (Example: "aw 2 916\n" makes the analog pin 5 set the analog value 916) NOTE: The DAC value range may depend on the client so, in case of using an Arduino board, the actual range will be 0-1023

"dr <pinNumber>\n": Digital read command. The client will read the digital value of the <pinNumber> assigned pin, and reply back with the read value concatenated to the actual command (Example: "dr 6\n" makes the client read the digital pin 6 value and reply "dr 6 0\n")

"dw <pinNumber> <value>\n": Digital write command. Sets the <pinNumber> assigned pin the digital on/off <value>. The client will reply with the same command if the execution was successful. (Example: "dw 5 1\n" makes the digital pin 5 set the digital value 1)
```


Related files:
```
ethernet_tcp_test_server.cpp: Code necessary to make the IOT2040 work as TCP server and send commands to the client (Arduino board) periodically
ethernet_tcp_test_client.ino: Code necessary to make the Arduino board work as TCP client and receive the commands sent by the server
```



