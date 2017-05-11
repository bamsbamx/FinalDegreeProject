#ifndef TinkerKit2_h
#define TinkerKit2_h

#include <stdint.h>
#include "mraa.hpp"

#define I0 0
#define I1 1
#define I2 2
#define I3 3
#define I4 4
#define I5 5

#define O0 11
#define O1 10
#define O2 9
#define O3 6
#define O4 5
#define O5 3

#define TK_MAX 1023
#define TK_X1 0	// 1x TKGyro model
#define TK_X4 1	// 4x TKGyro model
#define NORTH 1	// north pole in TKHallSensor
#define SOUTH 0	// south pole in TKHallSensor

// Arduino core libs stuff
#define LOW  0x0
#define HIGH 0x1

typedef uint8_t boolean;

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


/* 
-----------------------------------------------------------------------------
                                Generic Classes
-----------------------------------------------------------------------------
*/

class TKInputDigital {

	public:
		TKInputDigital(uint8_t _pin);
		boolean read();

	protected:
		mraa::Aio* mPin;

};


class TKInputAnalog {

	public:
		TKInputAnalog(uint8_t _pin);
		int read();

	private:
		mraa::Aio* mPin;

};

class TKInputAnalog2 {

	public:
		TKInputAnalog2(uint8_t _pinX, uint8_t _pinY);
		TKInputAnalog2(uint8_t _pinX, uint8_t _pinY, uint8_t _pinZ);
		int readX();
		int readY();
		int readZ();

	protected:
		mraa::Aio *mPinX, *mPinY, *mPinZ;

};

class TKOutput {

	public:
		TKOutput (uint8_t _pin);
		void write(float value);
		inline int state() { return _state; }
		void on() {
			write(1023);
			_state = HIGH;
		}
		void off() {
			write(0);
			_state = LOW;
		}

	protected:
		int _state;
		mraa::Pwm* mPin;
};

/*
 -----------------------------------------------------------------------------
                                Digital Inputs
 -----------------------------------------------------------------------------
*/


/*      Button      */
class TKButton: public TKInputDigital {

	public:
		TKButton(uint8_t _pin);
};


/*      Tilt Sensor     */
class TKTiltSensor: public TKInputDigital {

	public:
		TKTiltSensor(uint8_t pin);

};

/*      Touch Sensor        */
class TKTouchSensor : public TKButton {

	public:
		TKTouchSensor(uint8_t _pin);

};

/*
 -----------------------------------------------------------------------------
                                Analog Inputs
 -----------------------------------------------------------------------------
 */

/*      Potentiometer        */
class TKPotentiometer: public TKInputAnalog {

	public:
		TKPotentiometer(uint8_t pin);
		int read();
		int readStep(int steps);

	protected:
		// mraa::Aio->read() returns values from 0 to 1023
		const static int _minVal = 0;
		const static int _maxVal = 1023;

};


/*      Light Sensor        */
class TKLightSensor : public TKInputAnalog {

    public:
        TKLightSensor(uint8_t _pin);

};

/*      Temperature Sensor        */
class TKThermistor : public TKInputAnalog {

	public:
		TKThermistor(uint8_t _pin);
		float readCelsius();
		float readFahrenheit();

	protected:
		constexpr static float ADCres = 1023.0;
		const static int Beta = 3950;			// Beta parameter
		constexpr static float Kelvin = 273.15;	// 0°C = 273.15 K
		const static int Rb = 10000;			// 10 kOhm
		constexpr static float Ginf = 120.6685;	// Ginf = 1/Rinf
		// Rinf = R0*e^(-Beta/T0) = 4700*e^(-3950/298.15)

};

/*      Hall Sensor        */
class TKHallSensor : public TKInputAnalog {

	public:
		TKHallSensor(uint8_t _pin);
		boolean polarity();

	protected:
		const static uint16_t _zeroValue = 512;

};


/*      Joystick        */
class TKJoystick : public TKInputAnalog2 {

	public:
		TKJoystick(uint8_t _pinX, uint8_t _pinY);
		int readX();
		int readY();

};


/*      Gyro Sensor        */
class TKGyro : public TKInputAnalog2 {

	public:
		TKGyro(uint8_t _pinX, uint8_t _pinY, boolean model);
		void calibrate();
		long readXAxisRate();
		long readYAxisRate();

	protected:
		boolean model;

		//const static int _ADCresolution = 4880;	// [mV/count]	multiplied by 1000 to avoid float numbers
		// minimum sensitivity for the 1x module value (from datasheet is 0.167 mV/deg/s but the TinkerKit module has the outputs amplified 2x)
		//const static int _sensitivity = 334;	// Sensitivity is expressed in mV/degree/seconds, multiplied by 1000 to avoid float numbers.
		// This value represent the sensitivity of the 1x module. The sensitivity of the 4x module is 4x of this one
		long _sensitivityInCount;	// we obtain the sensitivity expressed in ADC counts
		// [counts/dps]
		int _yZeroVoltage;
		int _xZeroVoltage;

};

/*      Accelerometer        */
class TKAccelerometer : public TKInputAnalog2 {
	public:
		TKAccelerometer(uint8_t _pinX, uint8_t _pinY);
		TKAccelerometer(uint8_t _pinX, uint8_t _pinY, uint8_t _pinZ);
		inline float readXinG() { return (float)(readX() - _zeroOffset)/96; }
		inline float readYinG() { return (float)(readY() - _zeroOffset)/96; }
		inline float readZinG() { return (float)(readZ() - _zeroOffset)/96; }
		int inclination();

	protected:
		constexpr static float _gain = 1.414;
		const static int _zeroOffset = 478;

};

/*
 -----------------------------------------------------------------------------
                                    Outputs
 -----------------------------------------------------------------------------
 */

/*      LED     */
class TKLed : public TKOutput {

	public: 
        TKLed(uint8_t _pin);
        inline void brightness(int value) { write(value); }

};
		
/*      MosFet      */
class TKMosFet : public TKOutput {

	public:
		TKMosFet(uint8_t _pin);

};

/*      Relay       */
class TKRelay : public TKOutput {

	public:
		TKRelay(uint8_t _pin);

};


float map(float x, float in_min, float in_max, float out_min, float out_max);


#endif

