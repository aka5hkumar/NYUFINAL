#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
// GSR sensor variables
int sensorPin = 9; // select the input pin for the GSR
int sensorValue; // variable to store the value coming from the sensor
int ledValue;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(5, 8);

// Time variables
unsigned long time;
int secForGSR;
int curMillisForGSR;
int preMillisForGSR;
int threshold = 60;
int oldvalue = 0;
int newvalue = 0;
unsigned long oldmillis = 0;
unsigned long newmillis = 0;
int cnt = 0;
int timings[16];


void setup() {
// Prepare serial port
Serial.begin(9600);
secForGSR = 1; // How often do we get a GSR reading
curMillisForGSR = 0;
preMillisForGSR = -1;
strip.begin();
strip.show(); // Initialize all pixels to 'off'
}
void loop() {
time = millis();

curMillisForGSR = time / (secForGSR * 1000);
if(curMillisForGSR != preMillisForGSR) {
// Read GSR sensor and send over Serial port
sensorValue = analogRead(sensorPin);
ledValue=(sensorValue*5);
strip.setPixelColor(0, ledValue, ledValue, ledValue);
    strip.show();
Serial.print("GSR ");
Serial.print(sensorValue);
Serial.print('\n');
preMillisForGSR = curMillisForGSR;
}
  oldvalue = newvalue;
  newvalue = 0;
  for(int i=0; i<64; i++){ // Average over 16 measurements
    newvalue += analogRead(10);
  }
  newvalue = newvalue/64;
  // find triggering edge
  if(oldvalue<threshold && newvalue>=threshold){
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
    Serial.print("Heartrate ");
    Serial.println(heartrate,DEC);
    Serial.print('\n');
    strip.setBrightness(heartrate);
    strip.show();
    cnt++;
  }
  delay(5);
}
