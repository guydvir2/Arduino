//#include <LiquidCrystal.h>
//LiquidCrystal lcd(12,11,10,9,8,7);
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_mint; // Calculated litres/hour
unsigned char flowsensor = 2; // Sensor Input
unsigned long currentTime;
unsigned long cloopTime;
void flow () // Interrupt function
{
        flow_frequency++;
}
void setup()
{
        pinMode(flowsensor, INPUT);
        digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
        Serial.begin(9600);
        attachInterrupt(0, flow, RISING); // Setup Interrupt
        sei(); // Enable interrupts
        currentTime = millis();
        // lcd.begin(16, 2);
        cloopTime = currentTime;
//lcd.print(" 73/113/153/177");
        delay(2000);
//lcd.clear();
}
void loop ()
{
        currentTime = millis();
// Every second, calculate and print litres/mint
        if(currentTime >= (cloopTime + 1000))
        {
                cloopTime = currentTime; // Updates cloopTime
// Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
                l_mint = (flow_frequency * 1/ 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/mint
                flow_frequency = 0; // Reset Counter
                Serial.print(l_mint, DEC); // Print litres/hour
                Serial.println(" L/mint");
//lcd.setCursor(0,0);
//lcd.print(" Flow Meter");
//lcd.setCursor(0,1);
//lcd.setCursor(2,1);
//lcd.print(l_mint, DEC);
//lcd.setCursor(5,1);
//lcd.print("L/min");
        }
}
