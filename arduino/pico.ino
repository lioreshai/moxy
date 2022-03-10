#include <Adafruit_NeoPixel.h>
#include "Wire.h"
#include <DHT.h>
#include <DHT_U.h>

//Library for i2c connection
extern TwoWire Wire1;

//Set Neopixel parameters for panel meter lights
#define NEOPIXEL_PIN       2
#define NEOPIXEL_NUM_PIXELS 4

//Sensors
//DHT11 (temp + humidity)
#define DHTPIN 22
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
//MQ2
#define GAS_SENSOR_PIN 26
//Voltage divider - TODO: replace with a dedicated accurate voltage sensor
#define VOLTAGE_SENSOR_PIN 27

#define NUM_METERS 4
//Struct for panel meter state
typedef struct
{
    int meterPin;
    int ledPin;
    int currentLevel;
    bool highGood;
    int dataSource;
} PanelMeter ;

//Initialize panel meters
PanelMeter meters[NUM_METERS] = {
  {6, 3, 0, true, 0},
  {7, 2, 0, true, 0},
  {8, 1, 0, true, 0},
  {9, 0, 0, false, 0}
};

//Struct for color state
typedef struct {
  int r;
  int g;
  int b;
} RgbColor;

//Color scale - used to change color based on current meter level value
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

//Initialize neopixel
Adafruit_NeoPixel pixels(NEOPIXEL_NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//Initialize sensor state variables
uint8_t humidity=0;
uint8_t temperature=0;
uint8_t gasReading=0;
uint8_t voltageReading=0;

//Initialize meter update variable (updated by i2c request from Raspberry Pi)
const uint8_t METER_UPDATE_SIZE = 9;
volatile byte meterUpdate[METER_UPDATE_SIZE] = {0,0,0,0,0,0,0,0,0};

//Set up panel meter neopixels + output PWM pins
void setupMeters() {
  pixels.begin();
  for(int i=0; i<NUM_METERS; i++) {
    pinMode(meters[i].meterPin, OUTPUT);
  }
}

//Callback for data written by Raspberry Pi (meter state updates, Data IN)
void receiveEvent(int howMany){
  int i = 0;
  if(howMany==METER_UPDATE_SIZE) {
    while(Wire1.available()&&i<METER_UPDATE_SIZE) {
       meterUpdate[i]=Wire1.read();
       i++;
    }
  }
}

//Callback for sensor data request from Raspberry Pi (Data OUT)
void requestEvent() 
{
   Wire1.write(temperature);
   Wire1.write(humidity);
   Wire1.write(gasReading);
   Wire1.write(voltageReading);
}

void setup() {
    //Initialize i2c
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

//Parse stored payload from Raspberry Pi into panel meter struct
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

//Update meter PWM and neopixels based on values sent from Raspberry Pi
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

//Prepare sensor data to be available for request by Raspberry Pi
void preparePayloadForDataRequest() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    temperature = round(event.temperature);
    dht.humidity().getEvent(&event);
    humidity = round(event.relative_humidity);
    gasReading = analogRead(GAS_SENSOR_PIN);
    //Sample voltage reading - TODO: replace this with dedicated sensor, too inaccurate
    unsigned long voltageTotal = 0;
    for(int i=0; i<40; i++) {  
      voltageTotal += analogRead(VOLTAGE_SENSOR_PIN);
    }
    voltageReading = round(voltageTotal/40);
}

long lastMeterChange = 0;

void loop() {
    //non-blocking loop to update meters and read sensor data
  if(millis()-lastMeterChange > 400) {    
    updateMeters();
    preparePayloadForDataRequest();
    lastMeterChange = millis();
  }
}