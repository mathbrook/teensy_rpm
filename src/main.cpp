#include <Arduino.h>
#include <FreqMeasureMulti.h>
#include <Adafruit_ADS1X15.h>
#include "SdFat.h"

#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

#define RPM_TIMEOUT 100 // timeout on RPM to settle to zero 
// Measure 3 frequencies at the same time! :-)
FreqMeasureMulti freq1;
FreqMeasureMulti freq2;
FreqMeasureMulti freq3;
double current_rpm = 0;

float prev_rpm;
unsigned long current_rpm_change_time;
int sum = 0;
int count = 0;
unsigned long time1;
double current_rpm2 = 0;

float prev_rpm2;
unsigned long current_rpm_change_time2;
int sum2 = 0;
int count2 = 0;
unsigned long time2;
double current_rpm3 = 0;

float prev_rpm3;
unsigned long current_rpm_change_time3;
int sum3 = 0;
int count3 = 0;
unsigned long time3;
//adc variables
//space out bt using pins 0 and 3 to avoid short on board
int16_t adc0, adc3;
float volts0, volts3;
//string for writing to SD
String dataLine;
unsigned long log_rate=0;
#define SPI_CLOCK SD_SCK_MHZ(50)
//fuck
const int ledPin = 13;
// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS
#define SD_FAT_TYPE 1
#if SD_FAT_TYPE == 0
SdFat sd;
typedef File file_t;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
SdExFat sd;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
SdFs sd;
typedef FsFile file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE
#define FILE_BASE_NAME "BrakeTestLog"
file_t myFile;
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;
char fileName[] = FILE_BASE_NAME "00.txt";
//Stuff for talking to esp8266
void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  delay(10);

  // picking pin 5
  freq2.begin(6);
  freq1.begin(5);
  freq3.begin(9);
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      //Serial.println(F("Can't create file name"));
      return;
    }
  }
  if (!myFile.open(fileName, FILE_WRITE)) {
    sd.errorHalt("opening test.txt for write failed");
  }
  //myFile.open(fileName);
  myFile.println(fileName);
  myFile.println("Time,LeftWheel,RightWheel,OutputShaft,Brake1,Brake2");
   //Serial.begin(57600);
  //while (!Serial) ; // wait for Arduino Serial Monitor
  delay(10);
 // Serial.println("Getting single-ended readings from AIN0..3");
  //Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  //Serial.println("FreqMeasureMulti Begin");
  ads.setGain(GAIN_TWOTHIRDS);
  if (!ads.begin()) {
    //Serial.println("Failed to initialize ADS.");
    while (1);
  }
}

void loop() {
  digitalWrite(ledPin, LOW);
  adc0 = ads.readADC_SingleEnded(0);
  adc3 = ads.readADC_SingleEnded(3);

  volts0 = ads.computeVolts(adc0);
  volts3 = ads.computeVolts(adc3);

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
        float testRpm = freq1.countToFrequency(sum / count)*60 /18;
        current_rpm = (double)testRpm;

        /*if ( testRpm - prev_rpm < 1)
        {
          current_rpm = (double)testRpm;
          prev_rpm = testRpm;
        }*/
        // Serial.printf("Current RPM: %f \n", testRpm);
      sum = 0;
      count = 0;
      //prev_rpm = testRpm;
    }
  }
  unsigned long test2 = millis();
  if((test2 - current_rpm_change_time2) > RPM_TIMEOUT) {
    // rpm.data = 0;
    current_rpm2 = 0;

  }
  if (freq2.available()) {
    // average several reading together
    sum2 = sum2 + freq2.read();
    count2 = count2 + 1;
    current_rpm_change_time2 = millis();
    if (count2 > 1) {
        float testRpm2 = freq2.countToFrequency(sum2 / count2) *60/18;
        current_rpm2 = (double)testRpm2;
        /*if ( testRpm2 - prev_rpm2 < 1)
        {
          current_rpm2 = (double)testRpm2;
          prev_rpm2 = testRpm2;
        }*/
        // Serial.printf("Current RPM2: %f \n", testRpm);
      sum2 = 0;
      count2 = 0;
      prev_rpm2 = testRpm2;
    }
  }
   unsigned long test3 = millis();
  if((test3 - current_rpm_change_time3) > RPM_TIMEOUT) {
    // rpm.data = 0;
    current_rpm3 = 0;

  }
  if (freq3.available()) {
    // average several reading together
    sum3 = sum3 + freq3.read();
    count3 = count3 + 1;
    current_rpm_change_time3 = millis();
    if (count3 > 1) {
        float testRpm3 = freq3.countToFrequency(sum3 / count3) *60/20;
        current_rpm3 = (double)testRpm3;
        /*if ( testRpm2 - prev_rpm2 < 1)
        {
          current_rpm2 = (double)testRpm2;
          prev_rpm2 = testRpm2;
        }*/
        // Serial.printf("Current RPM2: %f \n", testRpm);
      sum3 = 0;
      count3 = 0;
      prev_rpm3 = testRpm3;
    }
  }
  /*if(time2<millis())
  {time2+=200;
  if (current_rpm2>-1){
    Serial.print("left wheel: ");
    Serial.println(current_rpm2);
  }
}
  if(time1<millis()){
    time1+=200;
  if (current_rpm>-1){
    Serial.print("right wheel:");
    Serial.println(current_rpm);
  }
}*/
float now = millis();
float timestamp = now/1000;
char buffer[80];
  int n;
  n=sprintf(buffer,"%f,%f,%f,%f,%f,%f\n",timestamp,current_rpm2,current_rpm,current_rpm3,volts0,volts3);
if(millis()-log_rate>=100){
  log_rate=millis();
  digitalWrite(ledPin, HIGH);
  
  
  //n=sprintf(buffer,"Time\t%f\tLeft wheel\t%f\tRight wheel\t%f\tBrake 1\t%fBrake 2\t%f\n",timestamp,current_rpm2,current_rpm,volts0,volts3);
  myFile.open(fileName,FILE_WRITE);
  myFile.write(buffer);
  myFile.close();
  Serial.print(buffer);
  }
}