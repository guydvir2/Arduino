#include <myIOT.h>
#include <Arduino.h>

// struct eeproms_storage
// {
//   byte cell_index;
//   byte value;
// };

// eeproms_storage boot_Counter = {0, 0};
// eeproms_storage totWrites_Counter = {1, 0};

// void zero_EEPROM_cell(int i)
// {
//   EEPROM.write(i, 0);
//   EEPROM.commit();
// }
// byte return_EEPROM_val(int i)
// {
//   return EEPROM.read(i);
// }
// void printValue(int i)
// {
//   Serial.print("Value of cell #");
//   Serial.print(i);
//   Serial.print(": ");
//   Serial.println(EEPROM.read(i));
// }
// bool check_hardReboot(byte threshold = 3)
// {
//   boot_Counter.value = EEPROM.read(boot_Counter.cell_index);
//   totWrites_Counter.value = EEPROM.read(totWrites_Counter.cell_index);

//   if (boot_Counter.value < threshold)
//   {
//     boot_Counter.value++;
//     totWrites_Counter.value++;
//     EEPROM.write(boot_Counter.cell_index, boot_Counter.value);
//     EEPROM.write(totWrites_Counter.cell_index, totWrites_Counter.value);
//     EEPROM.commit();
//     return 0;
//   }
//   else
//   {
//     return 1;
//   }
// }
hardReboot hardReboot;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  hardReboot.check_boot(2);
  // EEPROM.begin(64);
  // check_hardReboot();
  // delay(500);
  hardReboot.print_val(0);
  hardReboot.print_val(1);

  // printValue(0);
  // printValue(1);
}

void loop()
{
  if (millis() > 10000 && millis() < 10500)
  {
    Serial.println("Zeroing EEPROM");
    hardReboot.zero_cell(0);
  }
  delay(200);
  // put your main code here, to run repeatedly:
}
