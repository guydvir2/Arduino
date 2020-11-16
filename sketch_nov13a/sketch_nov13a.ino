
#include <EEPROM.h>

void EEPROMWritelong(int address, long value){

  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}
long EEPROMReadlong(long address){
    long four = EEPROM.read(address);
    long three = EEPROM.read(address + 1);
    long two = EEPROM.read(address + 2);
    long one = EEPROM.read(address + 3);

    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void increment(long address, byte amount){
  EEPROMWritelong(address, EEPROMReadlong(address)+amount);
}
void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(256);
  Serial.begin(9600);
  Serial.println("START!");
  for (int i = 0; i < 5;i++){
    EEPROMWritelong(i*4, i);
    EEPROMWritelong(i * 40, i*10);
    long x = EEPROMReadlong(i * 40);
    long y = EEPROMReadlong(i * 4);

    Serial.println(y);
    Serial.println(x);
  }

  // Serial.println(EEPROMReadlong(0));
  // EEPROMWritelong(4, 1234567);
  // Serial.println(EEPROMReadlong(4));
  // EEPROMWritelong(8, 9896789);
  // Serial.println(EEPROMReadlong(8));
}

void loop() {
  // put your main code here, to run repeatedly:

}
