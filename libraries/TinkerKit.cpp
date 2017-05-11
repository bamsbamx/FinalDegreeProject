#include <iostream>
#include <Math.h>      // PI
#include <TinkerKit.hpp>
#include <unistd.h>    // usleep()

TKInputDigital::TKInputDigital(uint8_t _pin) {
    mPin = NULL;
    mPin = new mraa::Aio(_pin);
    if (mPin == NULL) throw MRAA_ERROR_UNSPECIFIED;
}

TKInputAnalog::TKInputAnalog(uint8_t _pin) {
    mPin = NULL;
    mPin = new mraa::Aio(_pin);
	if (mPin == NULL) throw MRAA_ERROR_UNSPECIFIED;
}

TKInputAnalog2::TKInputAnalog2(uint8_t _pinX, uint8_t _pinY) {
	mPinX = NULL;
	mPinX = new mraa::Aio(_pinX);
	if (mPinX == NULL) throw MRAA_ERROR_UNSPECIFIED;
	mPinY = NULL;
	mPinY = new mraa::Aio(_pinY);
	if (mPinY == NULL) throw MRAA_ERROR_UNSPECIFIED;
	mPinZ = NULL;
}

TKInputAnalog2::TKInputAnalog2(uint8_t _pinX, uint8_t _pinY, uint8_t _pinZ) {
	mPinX = NULL;
	mPinX = new mraa::Aio(_pinX);
	if (mPinX == NULL) throw MRAA_ERROR_UNSPECIFIED;
	mPinY = NULL;
	mPinY = new mraa::Aio(_pinY);
	if (mPinY == NULL) throw MRAA_ERROR_UNSPECIFIED;
	mPinZ = NULL;
	mPinZ = new mraa::Aio(_pinZ);
	if (mPinZ == NULL) throw MRAA_ERROR_UNSPECIFIED;
}

TKOutput::TKOutput(uint8_t _pin) {
    _state = LOW;
	mPin = NULL;
	mPin = new mraa::Pwm(_pin);
	if (mPin == NULL) throw MRAA_ERROR_UNSPECIFIED;
	mPin->enable(true); // Initialise as output with low state
}


// Returns 0 or 1
boolean TKInputDigital::read() {
    return mPin->read() > 512 ? HIGH : LOW;
}

int TKInputAnalog::read() {
    return mPin->read(); // 0..1023
}

int TKInputAnalog2::readX() {
    int val = mPinX->read(); // 0..1023
    return val;
}

int TKInputAnalog2::readY() {
    return mPinY->read(); // 0..1023
}

int TKInputAnalog2::readZ() {
    return mPinZ->read(); // 0..1023
}

void TKOutput::write(float value) {
    if (value <= TK_MAX && value >= 0) {
    	float actualValue = map(value, 0, 1023, 0, 1.0f);
    	mPin->write(actualValue);
    } else throw;
}

/*      Button      */
TKButton::TKButton(uint8_t _pin) : TKInputDigital(_pin) {}

/*      Tilt Sensor         */
TKTiltSensor::TKTiltSensor(uint8_t _pin) : TKInputDigital (_pin){}

/*      Touch Sensor        */
 TKTouchSensor::TKTouchSensor(uint8_t _pin) : TKButton(_pin) {}


/*
 -----------------------------------------------------------------------------
                                Analog Inputs
 -----------------------------------------------------------------------------
 */

/*      Potentiometer       */
TKPotentiometer::TKPotentiometer(uint8_t _pin) : TKInputAnalog(_pin) {}

int TKPotentiometer::read() {
    return TKInputAnalog::read(); // From 0 to 1023
}

int TKPotentiometer::readStep(int steps) {
	return floor(map(read(), 0, 1023, 0, steps)); // From 0 to 'steps'

}
 
/*      Light Sensor        */
TKLightSensor::TKLightSensor(uint8_t _pin) : TKInputAnalog(_pin){}

/*      Temperature Sensor       */
TKThermistor::TKThermistor(uint8_t _pin) : TKInputAnalog(_pin) {}

float TKThermistor::readCelsius() {
	float Rthermistor = Rb * (ADCres / TKThermistor::read() - 1);
	float _temperatureC = Beta / (log( Rthermistor * Ginf )) ;
	return _temperatureC - Kelvin;
}

float TKThermistor::readFahrenheit() {
	float _temperatureF = (TKThermistor::readCelsius() * 9.0)/ 5.0 + 32.0;
	return _temperatureF;
}


/*      Hall Sensor     */
TKHallSensor::TKHallSensor(uint8_t _pin) : TKInputAnalog(_pin) {}

boolean TKHallSensor::polarity() {
	int value = read();
	if(value >= _zeroValue) return NORTH;
	else return SOUTH;
}

/*      Joystick        */
TKJoystick::TKJoystick(uint8_t _pinX, uint8_t _pinY) : TKInputAnalog2 (_pinX, _pinY) {}

int TKJoystick::readX() {
    return TKInputAnalog2::readX();
}

int TKJoystick::readY() {
    return TKInputAnalog2::readY();
}


/*      Gyro        */
TKGyro::TKGyro(uint8_t _pinX, uint8_t _pinY, boolean _model) : TKInputAnalog2 (_pinX, _pinY) {
 	_sensitivityInCount = 14633;  // 4.88mV / (0.167mV/dps * 2)
 	model = _model;
 	if (_model == TK_X4) _sensitivityInCount /= 4;
    // default values
    _xZeroVoltage = 503;	// 2.46V expressed in ADC counts
    _yZeroVoltage = 503;
}

void TKGyro::calibrate() {
	_xZeroVoltage = 0;	
	_yZeroVoltage = 0;

	for (uint8_t i=0; i<50; i++) {
		_yZeroVoltage += readY();
		_xZeroVoltage += readX();
		usleep(20 * 1000); // 20ms
    }
   _yZeroVoltage /= 50;	 
   _xZeroVoltage /= 50;	
}

long TKGyro::readXAxisRate() {
 	return ((long)(readX() - _xZeroVoltage) * _sensitivityInCount) / 1000;
}
 
long TKGyro::readYAxisRate() {
	return ((long)(readY() - _yZeroVoltage) * _sensitivityInCount) / 1000;
}
 
 
/*      Accelerometer        */
TKAccelerometer::TKAccelerometer(uint8_t _pinX, uint8_t _pinY) : TKInputAnalog2(_pinX,_pinY){}
TKAccelerometer::TKAccelerometer(uint8_t _pinX, uint8_t _pinY, uint8_t _pinZ) : TKInputAnalog2(_pinX,_pinY,_pinZ){}

int TKAccelerometer::inclination() {
	int xVal = readX() - _zeroOffset;
	int yVal = readY() - _zeroOffset;

	if(xVal <= 96 && yVal <= 96) {
		int inclination = atan2(xVal, yVal)*180/M_PI;
		return (int) inclination;
	} else return 0; //TODO Test
}

/*
-----------------------------------------------------------------------------
                                        Outputs
-----------------------------------------------------------------------------
*/

/*      LED      */
TKLed::TKLed(uint8_t _pin) : TKOutput(_pin) {}


/*      MosFet      */
TKMosFet::TKMosFet(uint8_t _pin) : TKOutput(_pin) {}


/*      Relay      */
TKRelay::TKRelay(uint8_t _pin) : TKOutput(_pin) {}

float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
