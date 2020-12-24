char *funt(char name[]){
  char name2[] = "guy";
  return name2;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop()
{
  // put your main code here, to run repeatedly:
  // fLog.looper();
  Serial.println(funt("boy"));
}
