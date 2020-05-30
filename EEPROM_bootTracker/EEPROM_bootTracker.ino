#include <EEPROM.h>
#include <Arduino.h>

struct eeproms_storage
{
  byte reset_count;
  byte total_count;
  bool hBoot;
};

eeproms_storage hReset_eeprom = {0, 0, false};

void zero_EEPROM_cell(int i)
{
  EEPROM.write(i, 0);
  EEPROM.commit();
}
byte return_EEPROM_val(int i)
{
  return EEPROM.read(i);
}

void check_hardReboot(byte threshold = 3)
{
  hReset_eeprom.reset_count = EEPROM.read(0);
  hReset_eeprom.total_count = EEPROM.read(1);
  // delay(100);
  // Serial.print("eeprom val: ");
  //         Serial.println(hReset_eeprom.val);

  if (hReset_eeprom.reset_count < threshold)
  {
    EEPROM.write(0, hReset_eeprom.reset_count + 1);
    EEPROM.write(1, hReset_eeprom.total_count + 1);
    EEPROM.commit();
    hReset_eeprom.reset_count = hReset_eeprom.reset_count + 1; //EEPROM.read(hReset_eeprom.val_cell);
    hReset_eeprom.hBoot = false;
  }
  else
  {
    hReset_eeprom.hBoot = true;
  }
}
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  EEPROM.begin(1024);
  check_hardReboot();
}

void loop()
{
  if (millis() > 10000)
  {
    Serial.println("Zeroing EEPROM");
  }
  // put your main code here, to run repeatedly:
}
