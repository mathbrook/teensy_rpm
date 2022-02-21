#include <Arduino.h>
#include <FreqMeasureMulti.h>
#define RPM_TIMEOUT 20 // timeout on RPM to settle to zero 
// Measure 3 frequencies at the same time! :-)
FreqMeasureMulti freq1;
double current_rpm = 0;

float prev_rpm;
unsigned long current_rpm_change_time;
int sum = 0;
int count = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(10);
  Serial.println("FreqMeasureMulti Begin");
  delay(10);

  // picking pin 5
  freq1.begin(5);
}

void loop() {

  unsigned long test = millis();
  if((test - current_rpm_change_time) > RPM_TIMEOUT) {
    // rpm.data = 0;
    current_rpm = 0;

  }
  
  if (freq1.available()) {
    // average several reading together
    sum = sum + freq1.read();
    count = count + 1;
    current_rpm_change_time = millis();
    if (count > 1) {
        float testRpm = freq1.countToFrequency(sum / count) * 28;
        if ( testRpm - prev_rpm < 1)
        {
          current_rpm = (double)testRpm;
          prev_rpm = testRpm;
        }
        // Serial.printf("Current RPM: %f \n", testRpm);
      sum = 0;
      count = 0;
      prev_rpm = testRpm;
    }
  }
  if(current_rpm > 0){
    Serial.println(current_rpm);
  }
  



    


}
