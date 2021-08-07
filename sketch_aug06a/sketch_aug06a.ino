// #include <myLOG.h>
// #include <ESP8266WiFi.h>

// flashLOG clklog("/clkLOG.txt");

// long T = 1628262500;
// void updatelog()
// {
//   char a[25];
//   sprintf(a, "%d", T + (int)(millis() / 1000));
//   clklog.write(a,true);
// }
int pin = D6;
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pin, OUTPUT);
  // clklog.start(5, 11, true, true);
  // updatelog();
}

void loop()
{
  // put your main code here, to run repeatedly:
  // static long lastW = 0;
  // clklog.looper(15);
  // if (millis() - lastW > 1500)
  // {
  //   lastW = millis();
  //   updatelog();
  //   clklog.rawPrintfile();
  // }
  int x = 0;
  while (x < 1023)
  {
    Serial.println(x);
    analogWrite(pin, x);
    delay(5);
    x = x + 1;
  }
  delay(50);
}
