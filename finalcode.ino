#include "photon_fft.h"
#include "Particle.h"
#include "neopixel.h"
#include <math.h>
#define FFTVal 256
#define PIXEL_PIN D2
#define PIXEL_COUNT 20
#define PIXEL_TYPE SK6812RGBW
#define BRIGHTNESS 250
std::vector<double> inputreal(FFTVal);
std::vector<double> inputimag(FFTVal);
int fftAVG;
SYSTEM_MODE(AUTOMATIC);
const int AUDIO_INPUT_PIN = DAC;
const int BOARD_LED_PIN = D7;
Fft call_fft;
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
String localip = "";
String subnetmask = "";
String gatewayip = "";
String ssid = "";
float FFT;
bool buffer = false;
int fftSize;
int ledcolor;
int soundChange;


void setup(){
  register int iter = 0;
  fftSize=0;
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show();
  pinMode(AUDIO_INPUT_PIN, INPUT);
  pinMode(BOARD_LED_PIN, OUTPUT);
  digitalWrite(BOARD_LED_PIN, HIGH);
  Serial.begin(115200);
  Serial.println("Starting Up, Hi!");
  localip = WiFi.localIP();
  Particle.variable("localip", localip);

  // grab the photon's subnet mask
  subnetmask = WiFi.subnetMask();
  Particle.variable("subnetmask", subnetmask);

  // grab the router's ip
  gatewayip = WiFi.gatewayIP();
  Particle.variable("gatewayip", gatewayip);

  // grab the network name
  ssid = WiFi.SSID();
  Particle.variable("ssid", ssid);
  startSampling();
}
void loop(){
  if (buffer==true){
    uint16_t analogIN = analogRead(AUDIO_INPUT_PIN);
    inputreal[fftSize] = analogIN;
    Particle.variable("Audio", analogIN);
    Particle.publish("Audio",String(analogIN), PRIVATE);
  	inputimag[fftSize] = 0;
    fftSize++;
  }
  if (fftSize>=FFTVal){
      stopSampling();
  }
}
void startSampling() {
  fftSize = 0;
  digitalWrite(BOARD_LED_PIN, HIGH);
  buffer = true;
    Serial.print("Sampling");
}

void stopSampling() {
  buffer = false;
  digitalWrite(BOARD_LED_PIN, LOW);
  processFFT();
}

void processFFT(){
  call_fft.transformRadix2(inputreal,inputimag);
  avgFFT();
  startSampling();
}
int avgout;
void avgFFT(){
  fftAVG=0;
  avgout=0;
  for (int iter = 0; iter < FFTVal; iter++) {

     // Serial.print("FFT: ");
     FFT=("real[%d]-%f , imag[%d]-%f\n",iter,inputreal[iter],iter,inputimag[iter]);
     // Serial.println(outputvar);
 // add samples to the string and separate them with spaces
     fftAVG=fftAVG+abs(FFT);

 }
 avgout=fftAVG/FFTVal;
 getLIT();
 }
void getLIT(){
    bool ledChange=false;
    int oldLED=ledcolor;
    uint32_t old = strip.getPixelColor(11);
  Serial.print("Average FFT: ");
  Serial.println(avgout);
  ledcolor=map(avgout, 0, 4095, 0 ,255);
  soundChange=abs(oldLED-ledcolor);
  oldLED=ledcolor;

    Serial.print("LED Change: ");
    Serial.println(soundChange);

   if(soundChange>10) {
       Serial.println(soundChange);
//       ledChange=true;
        ledColor();
   }
}
void ledColor(){
    Serial.print("LED MAP: ");
    Serial.println(ledcolor);
    if (soundChange > 50){
        colorWipe(strip.Color(255-ledcolor, 0, 0, soundChange), ledcolor/10); //Red

    }
    else if (soundChange >= 40){
        colorWipe(strip.Color(255, ledcolor, 0, soundChange), ledcolor/10); //Orange

    }
    else if (soundChange >= 30){
        colorWipe(strip.Color(255, 255-ledcolor, 0, soundChange), ledcolor/10); //???

    }
    else if (soundChange >= 20){
        colorWipe(strip.Color(0, 255-ledcolor, 255, soundChange), ledcolor/10); //Blue

    }
    else if (soundChange >= 10){
        colorWipe(strip.Color(0, 255, ledcolor, 50), ledcolor/10); //Teal

    }
    else  if (soundChange >= 5){
        colorWipe(strip.Color(0, 0, ledcolor, 150), ledcolor/10); //???

    }
    else{
     colorWipe(strip.Color(0, 0, 0, 150), 50); //???

    }
}
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
