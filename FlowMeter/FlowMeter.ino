
//  Pins to Flow_Meter
byte statusLed    = 13;
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;
// ######################

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;
volatile byte pulseCount;
float flowRate;
unsigned int flow_milLiters;
unsigned long total_milLitres;
unsigned long oldTime;

void setup()
{
        Serial.begin(38400);
        startGPIOs();


        pulseCount        = 0;
        flowRate          = 0.0;
        flow_milLiters    = 0;
        total_milLitres   = 0;
        oldTime           = 0;

        attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void startGPIOs(){
  pinMode(statusLed, OUTPUT);
  pinMode(sensorPin, INPUT);
  digitalWrite(statusLed, HIGH); // We have an active-low LED attached
  digitalWrite(sensorPin, HIGH);
}

void print_OL_readings(){
        unsigned int frac;
        // Print the flow rate for this second in litres / minute
        Serial.print("Flow rate: ");
        Serial.print(int(flowRate)); // Print the integer part of the variable
        Serial.print("."); // Print the decimal point
        // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
        frac = (flowRate - int(flowRate)) * 10;
        Serial.print(frac, DEC); // Print the fractional part of the variable
        Serial.print("L/min");
        // Print the number of litres flowed in this second
        Serial.print("  Current Liquid Flowing: "); // Output separator
        Serial.print(flow_milLiters);
        Serial.print("mL/Sec");

        // Print the cumulative total of litres flowed since starting
        Serial.print("  Output Liquid Quantity: "); // Output separator
        Serial.print(total_milLitres);
        Serial.println("mL");
}

void pulseCounter(){
        // Increment the pulse counter
        pulseCount++;
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           

void measureFlow(){
  if((millis() - oldTime) > 1000){ // calc cycle
          // Disable the interrupt while calculating flow rate
          detachInterrupt(sensorInterrupt);
          flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
          oldTime = millis();
          // convert to millilitres.
          flow = (flowRate / 60) * 1000;
          // Add the millilitres passed in this second to the cumulative total
          total_milLitres += flow_milLiters;

          print_OL_readings();
          // Reset the pulse counter- for next cycle
          pulseCount = 0;

          // Enable the interrupt again now that we've finished sending output
          attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

}


void loop(){
  measureFlow();
}
