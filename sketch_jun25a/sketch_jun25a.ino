// int Pins[]={34,35,32,33};
int Pins[]={A5,A4,A3,A2};
void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  for(int a=0; a<4; a++){
    pinMode(Pins[a], INPUT);
  }

}

void loop() {
  // put your main code here, to run repeatedly:
   for(int a=0; a<4; a++){
    // Serial.print("Sensor #");
    // Serial.print(a);
    // Serial.println(": ");
    Serial.print(analogRead(Pins[a]));
    Serial.print(";");
  }
  Serial.println("");
  delay(1000);
}
