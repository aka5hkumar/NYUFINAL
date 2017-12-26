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
  buffer = true;

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
     Serial.print("Average FFT: ")
     Serial.println(fftAVG);

 }
 avgout=fftAVG/FFTVal;
 getLIT();
 }
void getLIT(){
  int ledcolor;
  Serial.println(avgout);
  ledcolor=map(avgout, 0, 4095, 0 ,255);
  Serial.print("LED MAP: ")
  Serial.println(ledcolor);
}
