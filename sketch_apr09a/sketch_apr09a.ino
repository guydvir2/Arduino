// #include <EEPROM.h>
//
// void setup()
// {
//   for (int i = 0; i < 255; i++)
//     EEPROM.write(i, i);
// }
//
// void loop()
// {
// }


#include <EEPROM.h>

int a = 0;
int value;

void setup()
{
  Serial.begin(9600);
  // EEPROM.begin(512);
  //
  // for( int i=0; i<512; i++){
  //   EEPROM.write(i,i*2);
  // }
  // EEPROM.commit();
}

void loop()
{
  value = EEPROM.read(a);

  Serial.print(a);
  Serial.print("\t");
  Serial.print(value);
  Serial.println();

  a = a + 1;

  if (a == 512)
    a = 0;

  delay(500);
}
