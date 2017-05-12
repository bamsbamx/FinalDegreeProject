/*
 * This program contains the code to test the UART serial output (TX)
 * capability of the SIMATIC IOT2040.
 *
 * For this code the Sensor Shield together with a TinkerKit! potentiometer has been used.
 * The potentiometer value is being read in order to send its ADC value (returned
 * by the mraa library) 0 .. 1023 as a byte string via UART port provided by its
 * Arduino extension (pins 0 RX and 1 TX);
 */


#include <iostream>
#include "mraa.hpp"

#include <TinkerKit.hpp>


int main(void) {
	std::cout << "Hello, Serial Comm" << std::endl;

	mraa::Uart* serial;                // UART port
	TKPotentiometer pot(I0);           // TinkerKit! potentiometer

	try {
		serial = new mraa::Uart(0);    // Initialise UART port 0 (pins 0 and 1 of the Arduino extension)
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return mraa::ERROR_UNSPECIFIED;
	}

	// Set baud rate to 9600 bit/s
	if (serial->setBaudRate(9600) != mraa::SUCCESS) {
		std::cout << "Error setting baud rate" << std::endl;
		return mraa::ERROR_UNSPECIFIED;
	}

	//Set 8N1 mode, same as Arduino Serial.begin default value
	if (serial->setMode(8, mraa::UART_PARITY_NONE, 1) != mraa::SUCCESS) {
		std::cout << "Error setting parity" << std::endl;
		return mraa::ERROR_UNSPECIFIED;
	}

	if (serial->setFlowcontrol(false, false) != mraa::SUCCESS) {
		std::cout << "Error setting flow control" << std::endl;
	}


	char buffer [4];                   // The obtained String should not be more than 4 digits
	int potValue;
	while (true) {
		potValue = pot.read();         // Get ADC value from TinkerKit! potentiometer
		sprintf(buffer,"%d",potValue); // Convert to String
		serial->write(buffer, 4);      // Send value via UART port
		std::cout << "Potentiometer value: " << potValue << " sent: " << buffer << std::endl; // Debug pot. value
		usleep(1000*75);               // Wait 75 ms before next read
	}

	return 0;
}

