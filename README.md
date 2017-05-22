# FinalDegreeProject
This repo contains the source code files needed to accomplish the contents of my final degree proyect, based on the Siemens SIMATIC IOT2040 system dedicated to the industry IoT

Since the IOT2040 is based on the Intel Galileo GEN2, its SoC contains an Arduino expansion, making it compatible with the most Arduino shields. Thus, this project relies on the usage of such Arduino capability to implement a TinkerKit! called Arduino shield, in order to make sensor/actuator installation faster and easier. 



## Milestones

### 1.Create mraa compatible TinkerKit! library
The existing [official TinkerKit! library](https://github.com/TinkerKit/TinkerKit) is based on the Arduino core libs with the idea to be working just for Arduino. However, the IOT2040 works with [Intel´s C/C++ mraa library](https://iotdk.intel.com/docs/master/mraa/) so, to be able to get access to the TinkerKit! shield I/O pins, it is a must to make an adaptation of the [TinkerKit! code for Arduino](https://github.com/TinkerKit/TinkerKit) to make it work with Intel's [mraa](https://iotdk.intel.com/docs/master/mraa/) core libraries. That said, such adaptation can be found in the [/libraries](https://github.com/bamsbamx/FinalDegreeProject/tree/master/libraries) folder of this repo.

Related files:
```
libraries/TinkerKit.cpp: TinkerKit! library source file
libraries/TinkerKit.hpp: TinkerKit! header file
```
