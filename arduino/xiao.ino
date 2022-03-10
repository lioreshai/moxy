#include <Wire.h>

#define currentSense1 A7
#define currentSense2 A8
#define currentSense3 A9
#define currentSense4 A10

unsigned long lastCurrentSense = 0;

uint16_t current1 = 0;
uint16_t current2 = 0;
uint16_t current3 = 0;
uint16_t current4 = 0;

unsigned long current1Total;
unsigned long current2Total;
unsigned long current3Total;
unsigned long current4Total;

void sendAsWord(uint16_t data) {
  byte bigWord[2] = {highByte(data), lowByte(data)};
   Wire.write(bigWord, 2);
}

void requestEvent() 
{
   sendAsWord(current1);
   sendAsWord(current2);
   sendAsWord(current3);
   sendAsWord(current4);
}

void setup() {
  
  Serial.begin(9600);
  analogReadResolution(12);
  Wire.begin(9);
  Wire.onRequest(requestEvent);

}

void loop() {
  uint8_t sampleCount = 10;
  if(millis()-lastCurrentSense > 100) {
    current1Total=0;
    current2Total=0;
    current3Total=0;
    current4Total=0;
    for(int i=0; i<sampleCount; i++) {
      current1Total += analogRead(currentSense1);
      current2Total += analogRead(currentSense2);
      current3Total += analogRead(currentSense3);
      current4Total += analogRead(currentSense4);
    }
    current1 = round(current1Total/sampleCount);
    current2 = round(current2Total/sampleCount);
    current3 = round(current3Total/sampleCount);
    current4 = round(current4Total/sampleCount);
    lastCurrentSense = millis();
  }
}