uint8_t input_pins[4] = {19, 17, 5, 4};
uint8_t output_pins[4] = {25, 26, 33, 32};

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStart");

  for (uint8_t i = 0; i < 4; i++)
  {
    pinMode(output_pins[i], OUTPUT);
    digitalWrite(output_pins[i], LOW);
    delay(100);
    digitalWrite(output_pins[i], HIGH);
    delay(1000);
    digitalWrite(output_pins[i], LOW);
    delay(1000);
  }

  for (uint8_t i = 0; i < 4; i++)
  {
    pinMode(input_pins[i], INPUT_PULLUP);
  }
}

void loop()
{
  //    for (uint8_t i = 0; i < 4; i++)
  // {
  char a[20];
  sprintf(a, "%d;%d;%d;%d", digitalRead(input_pins[0]), digitalRead(input_pins[1]), digitalRead(input_pins[2]), digitalRead(input_pins[3]));
  Serial.println(a);
  delay(500);
  // }
}
