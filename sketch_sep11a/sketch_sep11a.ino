char *test[] = {&nullptr, nullptr};
void setup()
{
  Serial.begin(115200);
  Serial.println("START");
  char *a = "GUY)_SDVSDFVSDFVSDFVSDF";
  test[0] = a;
  Serial.println(test[0]);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(100);
}
