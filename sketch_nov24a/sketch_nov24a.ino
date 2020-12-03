#include <EEPROM.h>
void EEPROMWritelong(int address, long value)
{
    byte four = (value & 0xFF);
    byte three = ((value >> 8) & 0xFF);
    byte two = ((value >> 16) & 0xFF);
    byte one = ((value >> 24) & 0xFF);

    EEPROM.write(address, four);
    EEPROM.write(address + 1, three);
    EEPROM.write(address + 2, two);
    EEPROM.write(address + 3, one);
    EEPROM.commit();
}
long EEPROMReadlong(long address)
{
    long four = EEPROM.read(address);
    long three = EEPROM.read(address + 1);
    long two = EEPROM.read(address + 2);
    long one = EEPROM.read(address + 3);

    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Start");
  EEPROM.begin(100);
}

void loop() {
  // put your main code here, to run repeatedly:
    long t= millis();
    Serial.print("write: ");
    Serial.println(t);
    EEPROMWritelong(0, millis());
    Serial.print("read: ");
    Serial.println(EEPROMReadlong(0));
    delay(1000);
    

}
