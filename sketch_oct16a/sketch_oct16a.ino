/*  
  RCWL-0516 Latching Demo
  RCWL0516_latch_demo.ino
  Demonstrates using RCWL-0516 Microwave Sensor
  With pushbutton
  
  DroneBot Workshop 2019
  https://dronebotworkshop.com
*/

int Sensor = D2;     // RCWL-0516 Input Pin
int Pbut = 3;       // Push button Input Pin
int LED = BUILTIN_LED;       // LED Output Pin

int sensorval = 0;  // RCWL-0516 Sensor Value
int pbval = 0;      // Push button value


void setup() {
  Serial.begin(9600);
  pinMode (Sensor, INPUT);  // RCWL-0516 as input
  pinMode (Pbut, INPUT);    // Push button as input
  pinMode (LED, OUTPUT);    // LED as OUTPUT
  digitalWrite(LED, LOW);   // Turn LED Off
}

void loop(){

  sensorval = digitalRead(Sensor);  // Read Sensor value
  pbval = digitalRead(Pbut);        // Read Push button value
  
  if (sensorval == HIGH) {        
    digitalWrite(LED, HIGH);  // Turn LED On
    Serial.println("Detection");
  }
  
  if (pbval == LOW) {        
    digitalWrite(LED, LOW);  // Turn LED Off
  }
}
