/*
 * This Arduino sketch contains the code to test the UART serial output (TX)
 * capability of the SIMATIC IOT2040. The Arduino board MUST have more than
 * one UART ports in order for this sketch to work (has been tested with the 
 * Arduino ATMega 2560 board).
 * 
 * What this sketch does is basically to read the bytes passed by the SIMATIC 
 * IOT2040 through serial communication attached to PIN 19 (RX1) and parse
 * them to obtain a brightness value to light a LED via Pulse Width Modulation
 */
 
const int LED = 13; //Use the Arduino built-in LED

char inputChar;


void setup() {

  pinMode(LED, OUTPUT);
  
  Serial.begin(9600);    // Serial monitor (IDE input)
  Serial1.begin(9600);   // IOT2040 at pin19 (RX1). MUST HAVE THE SAME BAUD RATE AS TX!
}

void loop() {
  if (Serial1.available()) {
    delay(25);                          // Wait until all bytes are received
    
    String valueString = "";
    while (Serial1.available()) {
      inputChar = Serial1.read();       // Each char should be a digit '0' .. '9'
      valueString += inputChar;         // Concat the received char
    }
    
    float value = valueString.toInt();  // Input value must be 0 .. 1023 (eg. 50, 435 or 1013)
    value = value * 255 / 1024;         // Convert to 0 .. 255
    int brightness = round(value);      // Get more stable results
    analogWrite(LED, brightness);       // Light the LED with the calculated brightness
    
    Serial.print(valueString);          // Debug received value to the IDE Serial Monitor
  }
}


