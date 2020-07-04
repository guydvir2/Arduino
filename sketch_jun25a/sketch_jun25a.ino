int Pins[] = {34, 35, 32, 33};
const char *dirs[] = {"UP", "LEFT", "DOWN", "RIGHT"};
// int Pins[]={5,4,3,2};
void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);
  for (int a = 0; a < 4; a++)
  {
    pinMode(Pins[a], INPUT);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  for (int a = 0; a < 4; a++)
  {
    // Serial.print("Sensor #");
    // Serial.print(a);
    // Serial.println(": ");
    Serial.print(dirs[a]);
    Serial.print(":");
    Serial.print(analogRead(Pins[a]));
    Serial.print(";");
  }
  Serial.println("");
  Serial.print("Sides: ");
      Serial.print(analogRead(Pins[0]) - analogRead(Pins[2]));
  Serial.print("; ");
  Serial.print("Pitch: ");
      Serial.print(analogRead(Pins[1]) - analogRead(Pins[3]));
  Serial.println("");

  delay(1000);
}
