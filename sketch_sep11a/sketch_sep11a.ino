char *test[] = {nullptr, nullptr};
int a = 100;
int &p = a;
void setup()
{
  Serial.begin(115200);
  Serial.println("start");
  Serial.println(a);
  // Serial.println(&a);
  // Serial.println(*p);
  Serial.println(*&p);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(100);
}
