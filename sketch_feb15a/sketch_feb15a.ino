char *a="GUYDVIR";
char *b;
const int x=3;
char *c[x];
char d[40];
void setup()
{
  // put your setup code here, to run once:
//  testit.start();

  Serial.begin(115200);
  Serial.println("Start");
  b=a;
//  Serial.println(b);
strcpy(d,a);

}

void loop()
{
  delay(200);
  // long retVal;
  // Serial.println("GUY");
  // testit.setValue("millis", (long)millis());
  // testit.getValue("millis", retVal);
  // Serial.println(retVal);
//    Serial.println(b);
//for(int i=0;i<x;i++){
////  strcpy(c[i],a);
//  c[i]=a;
//  Serial.print(i);
//  Serial.print(": ");
//  Serial.println(c[i]);
//}
Serial.println(d);


}
