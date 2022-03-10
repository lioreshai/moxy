#include <Adafruit_NeoPixel.h>
#include "Wire.h"

#include <DHT.h>
#include <DHT_U.h>
extern TwoWire Wire1;
#define NEOPIXEL_PIN       2
#define NEOPIXEL_NUM_PIXELS 4

//Sensors
#define DHTPIN 22
#define DHTTYPE    DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
#define GAS_SENSOR_PIN 26
#define VOLTAGE_SENSOR_PIN 27

#define NUM_METERS 4
typedef struct
{
    int meterPin;
    int ledPin;
    int currentLevel;
    bool highGood;
    int dataSource;
} PanelMeter ;

PanelMeter meters[NUM_METERS] = {
  {6, 3, 0, true, 0},
  {7, 2, 0, true, 0},
  {8, 1, 0, true, 0},
  {9, 0, 0, false, 0}
};

typedef struct {
  int r;
  int g;
  int b;
} RgbColor;

RgbColor colorScale[10] = {
  {255, 20, 0},
  {255, 50, 0},
  {255, 80, 0},
  {255, 110, 0},
  {220, 255, 0},
  {200, 255, 0},
  {120, 255, 0},
  {80, 255, 10},
  {40, 255, 30},
  {0, 255, 50}
};

Adafruit_NeoPixel pixels(NEOPIXEL_NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500


const uint8_t METER_UPDATE_SIZE = 9;

uint8_t humidity=0;
uint8_t temperature=0;
uint8_t gasReading=0;
uint8_t voltageReading=0;

volatile byte meterUpdate[METER_UPDATE_SIZE] = {0,0,0,0,0,0,0,0,0};


void setupMeters() {
  pixels.begin();
  for(int i=0; i<NUM_METERS; i++) {
    pinMode(meters[i].meterPin, OUTPUT);
  }
}

void receiveEvent(int howMany){
  int i = 0;
  if(howMany==METER_UPDATE_SIZE) {
    while(Wire1.available()&&i<METER_UPDATE_SIZE) {
       meterUpdate[i]=Wire1.read();
       i++;
    }
  }
}

void requestEvent() 
{
   Wire1.write(temperature);
   Wire1.write(humidity);
   Wire1.write(gasReading);
   Wire1.write(voltageReading);
}

void setup() {

  Wire1.setSDA(10);
  Wire1.setSCL(11);
  Wire1.begin(8);
  Wire1.onRequest(requestEvent);
  Wire1.onReceive(receiveEvent);
  setupMeters();
  dht.begin();
  Serial.begin(9600);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(VOLTAGE_SENSOR_PIN, INPUT);
}

long lastMeterChange = 0;

void updateMeterObjectsFromI2CPayload() {
   //Example payload: [100 {command},222 {meter 1 value},1 {meter 2 HG},222 {...},0,222,1,222,0]
   if(meterUpdate[0]==100) {
    for(int i=1;i<=8;i++) {
      uint8_t meterObjectIndex = 0;
      if(i%2==0) {
        //Even = highGood setting
        meterObjectIndex = (i/2)-1;
        meters[meterObjectIndex].highGood = meterUpdate[i];
      } else {
        //Odd = value setting
        meterObjectIndex = ((i+1)/2)-1;
        if(meterUpdate[i]>240) {
          meterUpdate[i] = 240;
        }
        meters[meterObjectIndex].currentLevel = meterUpdate[i];
      }
    }
   }
}

void updateMeters() {
  updateMeterObjectsFromI2CPayload();
  for(int i=0; i<NUM_METERS; i++) {
    int colorLevel = floor((meters[i].currentLevel/240.00)*10.00);
    if(!meters[i].highGood) {
    colorLevel = 9-colorLevel;
  }
    pixels.setPixelColor(meters[i].ledPin, pixels.Color(colorScale[colorLevel].r,colorScale[colorLevel].g,colorScale[colorLevel].b));
    analogWrite(meters[i].meterPin, meters[i].currentLevel);
  }
  pixels.show();
}

void preparePayloadForDataRequest() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    temperature = round(event.temperature);
    dht.humidity().getEvent(&event);
    humidity = round(event.relative_humidity);
    gasReading = analogRead(GAS_SENSOR_PIN);
    unsigned long voltageTotal = 0;
    for(int i=0; i<40; i++) {  
      voltageTotal += analogRead(VOLTAGE_SENSOR_PIN);
    }
    voltageReading = round(voltageTotal/40);
}

void loop() {
  if(millis()-lastMeterChange > 400) {    
    updateMeters();
    preparePayloadForDataRequest();
    lastMeterChange = millis();
  }
}