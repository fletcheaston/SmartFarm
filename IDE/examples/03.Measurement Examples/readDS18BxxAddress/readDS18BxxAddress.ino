// This program is used to find DS18B20 Temperature Sensor address
// Based on the from http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

#include <OneWire.h>

void setup()
{
  Serial.begin(57600);
  Serial.println("Start Searching . . .");
  Serial.println();

  findDevices(3);
  Serial.println("Done!");
}

void loop()
{
}

uint8_t findDevices(int pin)
{
  OneWire ow(pin);
  uint8_t address[8];
  uint8_t count = 0;


  if (ow.search(address))
  {
    do {
      count++;
      Serial.print("{ ");
      for (uint8_t i = 0; i < 8; i++)
      {
        Serial.print("0x");
        if (address[i] < 0x10) Serial.print("0");
        Serial.print(address[i], HEX);
        if (i < 7) Serial.print(", ");
      }
      Serial.println(" };");
    } while (ow.search(address));

    Serial.print("# of devices found: ");
    Serial.println(count);
    Serial.println();

  }

  return count;
}
