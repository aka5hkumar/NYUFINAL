

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
int neopin= 9;
int GSRpin = 10;
int HRpin = 6;
int TEMPpin = 12;
int initTemp;
int GSRSense;
int tempSense;
int ledVal;
unsigned long time;
int GSRpoll;
int curMillisForGSR;
int preMillisForGSR;
int threshold = 60;
int initHeart = 0;
int hrSense = 0;
unsigned long oldmillis = 0;
unsigned long newmillis = 0;
int cnt = 0;
int timings[16];
int numled=4;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, neopin, NEO_RGBW + NEO_KHZ800);

void setup(){
  Serial.begin(9600);
  strip.begin();
  strip.setBrightness(100);
  strip.show();
  GSRpoll = 1;
  curMillisForGSR = 0;
  preMillisForGSR = -1;
  initTemp=analogRead(TEMPpin);
  Serial.print("Initial Temp Reading");
  Serial.println(initTemp);
}

void loop(){
  time = millis();
  curMillisForGSR = time / (GSRpoll *1000);
  tempSense= analogRead(TEMPpin);
  Serial.print('Temp ');
  Serial.println(tempSense);
  hrSense=analogRead(HRpin);
  Serial.print('Heart Rate ');
  Serial.println(hrSense);
    GSRSense = analogRead(GSRpin);
    Serial.print("GSR ");
    Serial.println(GSRSense);
    colorWipe(strip.Color(255, 0, 255), 5);
    preMillisForGSR = curMillisForGSR;
    if (tempSense+5>=initTemp)
    {
      Serial.print("Temp High ");
      for(uint16_t i=0; i<strip.numPixels(); i++)
      {
        uint32_t oldcolor = strip.getPixelColor(1);
        strip.setPixelColor(i, 255,0,0);
        delay(1500-tempSense*10);
        strip.show();
        strip.setPixelColor(i, oldcolor);
        strip.show();
      }
    }
    if (tempSense-5<=initTemp)
    {
      Serial.print("Temp Low ");
      uint32_t oldcolor = strip.getPixelColor(1);
      for(uint16_t i=0; i<strip.numPixels(); i++)
      {
        strip.setPixelColor(i, 0,255,0);
        delay(1500-tempSense*10);
        strip.show();
        strip.setPixelColor(i, oldcolor);
        strip.show();
      }
  }

  initHeart = hrSense;
  hrSense=0;
  for(int i=0; i<64; i++)
  { // Average over 16 measurements
    hrSense += analogRead(HRpin);
  }
  hrSense= hrSense/64;
  if(initHeart<threshold && hrSense>=threshold){
      oldmillis = newmillis;
      newmillis = millis();
      // fill in the current time difference in ringbuffer
      timings[cnt%16]= (int)(newmillis-oldmillis);
      int totalmillis = 0;
      // calculate average of the last 16 time differences
      for(int i=0;i<16;i++){
        totalmillis += timings[i];
      }
      // calculate heart rate
      int heartrate = 60000/(totalmillis/16);
      Serial.println(heartrate,DEC);
      cnt++;
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 255,0,0);
        strip.show();
      }
  delay(heartrate/60);
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
