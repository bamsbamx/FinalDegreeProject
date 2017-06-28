#include <SPI.h>
#include <Ethernet.h>

// Client config (this board)
const byte MAC_ADDRESS[] = {0x90, 0xA2, 0xDA, 0x0D, 0xBF, 0x61};
const byte IP_ADDRESS[] = {192, 168, 1, 143};
const byte GATEWAY[] = {192, 168, 1, 140};
const byte SUBNET[] = {255, 255, 255, 0};

// Server config (Simatec IOT2040)
const byte SERVER_IP_ADDRESS[] = {192, 168, 1, 140};
const int SERVER_PORT = 55056;

const int MAX_LENGTH = 256;

EthernetClient client;


void setup() {
	
  Serial.begin(9600);

  for (int i = 0; i < 9; i++) pinMode(i, OUTPUT);

  Serial.println("Initializing client...");
  // Try DHCP connection
  if (Ethernet.begin(MAC_ADDRESS) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Try using static IP address instead of DHCP
    Ethernet.begin(MAC_ADDRESS, IP_ADDRESS, GATEWAY, SUBNET);
  }
  
  delay(500);
}


void loop() {

  // Wait until server becomes online
  while (!client.connect(SERVER_IP_ADDRESS, SERVER_PORT)) {
    Serial.println("Server not found, retrying...");
    delay(1000);
  }

  // At this time, server is online and client (this board) is connected to it
  Serial.println("Client connected");
  char command[MAX_LENGTH];
  int i = 0;
  char receivedByte;

  // Keep reading inputs while client is connected
  while (client.connected()) {
    // Read received bytes if available
    while (client.available()) {
      receivedByte = client.read();
      switch (receivedByte) {
        case 10: // char 10 means line end (\n), which means the end of command
          Serial.print("Command received, parsing: ");
          Serial.println(command);
          command[i] = 0; // Add string terminator at the end
          parseCommand(command);
          i = 0; // Reset counter
          break;
        default:
          if (i >= MAX_LENGTH) {
            reportError("Maximum byte count reached");
            i = 0;
          }
          // Add read byte into buffer
          command[i] = receivedByte;
          delayMicroseconds(1);
          i++;
      }
    }
  }
  client.stop();
}


void parseCommand(char* command) {

  // Extract command arguments
  char commandName[2];
  int pin;
  int value;
  int n = sscanf(command, "%s %d %d", commandName, &pin, &value);

  if (n == 0) {
    reportError("No command received"); //Unknown command
    return;
  }
  
  // ANALOG WRITE COMMAND
  if (strcmp(commandName, "aw") == 0) {
    if (n != 3 || value < 0 || value > 255) {
      reportError("Usage: aw <pinNumber(3,5,6,9)> <analogicValue(0-255)>");
      return;
    }
    if (pin != 3 && pin != 5 && pin != 6 && pin != 9) {
      reportError("PWM available pins are digital 3,5,6,9");
      return;
    }
    analogWrite(pin, value);
    client.println(command);
  } 
  // ANALOGIC READ COMMAND
  else if (strcmp(commandName, "ar") == 0) {
    if (n != 2 || pin < 0 || pin > 5) {
      reportError("Usage: ar <pinNumber(0-5)>");
      return;
    }
    // First analog pin A0 is equivalent to 14. See: https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/variants/standard/pins_arduino.h#L56
    int value = analogRead(pin + 14); 
    // Reply with read value
    client.print("ar ");
    client.print(pin, DEC);
    client.print(" ");
    client.println(value, DEC);
  } 
  // DIGITAL WRITE COMMAND
  else if (strcmp(commandName, "dw") == 0) {
    if (n != 3 || value < 0 || value > 1 || pin < 0 || pin > 9) {
      reportError("Usage: dw <pinNumber> <digitalValue(0-1)>");
      return;
    }
    if (pin == 4) {
      reportError("Digital pin 4 is reserved for the Arduino ethernet shield");
      return;
    }
    digitalWrite(pin, value == 1 ? HIGH : LOW);
    client.println(command);
  } 
  // DIGITAL READ COMMAND
  else if (strcmp(commandName, "dr") == 0) {
    if (n != 2 || pin < 0 || pin > 9) {
      reportError("Usage: dr <pinNumber(0-9)>");
      return;
    }
    if (pin == 4) {
      reportError("Digital pin 4 is reserved for the Arduino ethernet shield");
      return;
    }
    int state = digitalRead(pin);
    // Reply with read value
    client.print("dr ");
    client.print(pin, DEC);
    client.print(" ");
    client.println(state == HIGH ? 1 : 0, DEC);
  } 
  // UNKNOWN COMMAND
  else {
    reportError("Unknown command. Available commands: dr, dw, ar, aw");
    return;
  }
}


// Reply with error to the server
void reportError(char* error) {
  Serial.println(error);
  if (client.connected()) {
    client.print("ERROR: ");
    client.println(error);
  }
}



