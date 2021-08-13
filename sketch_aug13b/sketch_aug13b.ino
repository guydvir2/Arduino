#include <myJSON.h>
myJSON test("/test.json", true);
int x = 0;
long q=1234567890;
bool b=false;

char retVal[20];
void setup()
{
  Serial.begin(115200);
  test.start();
  test.setValue("key0", 1234);
  test.getValue("key0", x);
  Serial.println(x);

  test.setValue("key1", "abcdefg");
  test.getValue("key1", retVal);

  test.setValue("key2", 123456788);
  test.getValue("key2", q);

  test.setValue("key3", true);
  test.getValue("key1", b);

  test.printFile();
}

void loop()
{
  delay(100);
}
