char topic_sub[3][40];

void func(static int i, char _array[i], const char* msg) {

  //    sprintf(_array, msg);
  //  _array = 'a';.
}


void setup()
{
  Serial.begin(115200);
  Serial.println("\nStart");

  sprintf(topic_sub[0], "GUY_DVIR");
  sprintf(topic_sub[1], "OZ_DVIR");
  sprintf(topic_sub[2], "OR_DVIR");

  func(topic_sub[0], "GUY");
  Serial.println(topic_sub[0]);

}

void loop()
{
}
