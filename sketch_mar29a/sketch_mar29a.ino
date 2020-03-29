int relPin = D3;
int detPin = D7;

void setup() {
  // put your setup code here, to run once:
  pinMode (relPin, OUTPUT);
  pinMode(detPin, INPUT);
         
  Serial.begin(9600);
  Serial.println("BEGIN");

}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println(digitalRead(detPin));
  digitalWrite(relPin, 1);
  delay(2000);
  digitalWrite(relPin, 0);
  delay(2000);
  
  
  

}
