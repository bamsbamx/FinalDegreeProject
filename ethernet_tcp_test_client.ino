#include <SPI.h>
#include <Ethernet.h>

// Client config
const byte MAC_ADDRESS[] = {0x90, 0xA2, 0xDA, 0x0D, 0xBF, 0x61};
const byte IP_ADDRESS[] = {192, 168, 1, 143};
const byte GATEWAY[] = {192, 168, 1, 140};
const byte SUBNET[] = {255, 255, 255, 0};

// Server config
const byte SERVER_IP_ADDRESS[] = {192, 168, 1, 140};
const int SERVER_PORT = 55056;

const int MAX_LENGTH = 256;

EthernetClient client;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 9; i++) pinMode(i, OUTPUT);

  Serial.println("Initializing client...");
  // TODO Fill readme and SIMATEC IOT2000 ip address setup
  // TODO Try using DHCP
  //if (Ethernet.begin(MAC_ADDRESS) == 0) {
  //Serial.println("Failed to configure Ethernet using DHCP");
  // try to congifure using IP address instead of DHCP:
  Ethernet.begin(MAC_ADDRESS, IP_ADDRESS, GATEWAY, SUBNET);
  //}
  delay(1000);
}


void loop() {

  while (!client.connect(SERVER_IP_ADDRESS, SERVER_PORT)) {
    Serial.println("Server not found, retrying...");
    delay(1000); //Wait until the server is connected
  }

  Serial.println("Client connected");
  char command[MAX_LENGTH];
  int i = 0;
  char receivedByte;

  while (client.connected()) {
    while (client.available()) {
      receivedByte = client.read();
      switch (receivedByte) {
        case 10: // char 10 means line end (\n)
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
          command[i] = receivedByte;
          delayMicroseconds(1);
          i++;
      }
    }
    //delay(100);
  }
  client.stop();
}


void parseCommand(char* command) {

  char commandName[2];
  int pin;
  int value;
  int n = sscanf(command, "%s %d %d", commandName, &pin, &value);
  int mustAnalogWrite = 0;

  if (n == 0) {
    reportError("No command received"); //Unknown command
    return;
  }
  if (strcmp(commandName, "aw") == 0) {
    if (n != 3 || value < 0 || value > 255) {
      reportError("Usage: aw <pinNumber(3,5,6,9)> <analogicValue(0-255)>");
      return;
    }
    if (pin != 3 && pin != 5 && pin != 6 && pin != 9) {
      reportError("PWM available pins are digital 3,5,6,9");
      return;
    }
    //analogWrite(pin, value);
    mustAnalogWrite = 1;
    client.println(command);
  } else if (strcmp(commandName, "ar") == 0) {
    if (n != 2 || pin < 0 || pin > 5) {
      reportError("Usage: ar <pinNumber(0-5)>");
      return;
    }
    int value = analogRead(pin + 14); // First analog pin A0 is equivalent to 14. See: https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/variants/standard/pins_arduino.h
    client.print("ar ");
    client.print(pin, DEC);
    client.print(" ");
    client.println(value, DEC);
  } else if (strcmp(commandName, "dw") == 0) {
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
  } else if (strcmp(commandName, "dr") == 0) {
    if (n != 2 || pin < 0 || pin > 9) {
      reportError("Usage: dr <pinNumber(0-9)>");
      return;
    }
    if (pin == 4) {
      reportError("Digital pin 4 is reserved for the Arduino ethernet shield");
      return;
    }
    int state = digitalRead(pin);
    client.print("dr ");
    client.print(pin, DEC);
    client.print(" ");
    client.println(state == HIGH ? 1 : 0, DEC);
  } else {
    reportError("Unknown command. Available commands: dr, dw, ar, aw");
    return;
  }
  if (pin == 3) analogWrite(pin, value);
}

void reportError(char* error) {
  Serial.println(error);
  if (client.connected()) {
    client.print("ERROR: ");
    client.println(error);
  }
}



