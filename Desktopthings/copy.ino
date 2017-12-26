// This #include statement was automatically added by the Particle IDE.

// This #include statement was automatically added by the Particle IDE.
#include "photon_fft.h"

// This #include statement was automatically added by the Particle IDE.
#include "Particle.h"
#include "neopixel.h"
#define R 512


/* ======================= prototypes =============================== */
std::vector<double> inputreal(R);
std::vector<double> inputimag(R);
uint32_t Wheel(byte WheelPos);
uint8_t red(uint32_t c);
uint8_t green(uint32_t c);
uint8_t blue(uint32_t c);
void colorWipe(uint32_t c, uint8_t wait);
void pulseWhite(uint8_t wait);
void rainbowFade2White(uint8_t wait, int rainbowLoops, int whiteLoops);
void whiteOverRainbow(uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength);
void fullWhite();
void rainbowCycle(uint8_t wait);
void rainbow(uint8_t wait);

/* ======================= rgbw-strandtest.cpp ====================== */

SYSTEM_MODE(AUTOMATIC);

#include <math.h>

// DEFINES =====
#define FFT_FORWARD 1
#define FFT_REVERSE -1



// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D2
#define PIXEL_COUNT 20
#define PIXEL_TYPE SK6812RGBW
#define BRIGHTNESS 250 // 0 - 255
// PINS =====
const int AUDIO_INPUT_PIN = DAC;
const int BOARD_LED_PIN = D7;


Fft call_fft;
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// TCP INITIALIZATION =====

TCPServer server = TCPServer(23);
TCPClient client;

String sendString;

String localip = "";
String subnetmask = "";
String gatewayip = "";
String ssid = "";

// FFT INITILIZATION =====
// set the sizes for the FFT
const int m = 8;
const int FFT_SIZE = R;

// initialize the buffers for the FFT
int samples;
int sampleCounter = 0;
float fftOUT=0;
int outputvar;
int avgout;

// flag that lets us know when we should or shouldn't fill the buffer
bool fillBuffer = true;


// DEFAULT FUNCTIONS =====

void setup() {
    register int i = 0;
     Particle.subscribe("hook-response/FFTarray", myHandler, MY_DEVICES);

    Particle.variable("FFT", FFT_SIZE);
    strip.setBrightness(BRIGHTNESS);
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    // Set up ADC and audio input.
    pinMode(AUDIO_INPUT_PIN, INPUT);

    // Turn on the power indicator LED.
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite(BOARD_LED_PIN, HIGH);

    // serial can be read with `screen ADDRESS BAUD`
    // where the ADDRESS can be found with `ls /dev/tty.usb*`
    // and the baud rate is 115200
    Serial.begin(115200);
    Serial.println("Starting Up, Hi!");


    // set up tcp connection by grabbing variables from the cloud
    sendString.reserve(20);

    // grab the photon's IP
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

    // start listening for clients
    server.begin();


    // begin sampling the audio pin
    startSampling();

}

double audioMAP;
void loop() {
    // colorWipe(strip.Color(255, 0, 0), 50); // Red
    // colorWipe(strip.Color(0, 255, 0), 50); // Green
    // colorWipe(strip.Color(0, 0, 255), 50); // Blue
    // colorWipe(strip.Color(0, 0, 0, 255), 50); // White


  // if we're currently filling the buffer
  if (fillBuffer == true) {
    //   Serial.print("MicLevel: ");
      uint16_t value = analogRead(AUDIO_INPUT_PIN);
      audioMAP = map(value, 0, 4095, 0, 255);
      Particle.variable("Sound", audioMAP);
      Serial.print("Raw Audio");
      Serial.println(value);
    // read from the input pin and start filling the buffer
    inputreal[sampleCounter] = value;
    digitalWrite(BOARD_LED_PIN, LOW);
    //Serial.println(audioMAP);

    // the buffer full of imaginary numbers for the FFT doesn't matter for our use, so fill it with 0s
    inputimag[sampleCounter] = 0.0;
    // Serial.print("Sample: ");
    // Serial.println(sampleCounter);
    // increment the counter so we can tell when the buffer is full
    sampleCounter++;

  }
    int tempavg;
  // if the buffer is full
  if (samplingIsDone()) {

    // stop sampling
    stopSampling();
    //triggerLeds();
    // do the FFT
      for (int i = 0; i < FFT_SIZE; i++) {
        //   Serial.print("FFT: ");
          outputvar=("real[%d]-%f , imag[%d]-%f\n",i,inputreal[i],i,inputimag[i]);
        //   Serial.println(outputvar);
      }
    client.connect("165.227.72.208", 9000);
    // char mode = sendData();
    // Serial.print("mode: ");
    // Serial.println(mode);
    tempavg=avgData();
    // Serial.print("FFTAVG: ");
    // Serial.println(tempavg);
    Particle.variable("FFTAVG", tempavg);
    delay(100);

  }

}
char avgData(){
         for (int i = 0; i < FFT_SIZE; i++) {
            // Serial.print("FFT: ");
            outputvar=("real[%d]-%f , imag[%d]-%f\n",i,inputreal[i],i,inputimag[i]);
            // Serial.println(outputvar);
        // add samples to the string and separate them with spaces
            samples=samples+abs(outputvar);
}
    avgout=samples/R;
    return avgout;
}
char sendData(){
    // if the client is connected to the server
      // initialize an empty string, so we can fill it with data
      sendString = "";

    //  sendString += "START ";
      //sendString += " ";
      for (int i = 0; i < FFT_SIZE; i++) {
            Serial.print("FFT: ");
            outputvar=("real[%d]-%f , imag[%d]-%f\n",i,inputreal[i],i,inputimag[i]);
            Serial.println(outputvar);
        // add samples to the string and separate them with spaces
            sendString += String(outputvar);
            samples+=outputvar;


      //  sendString += String(i * 2);
            sendString += ",";
      }
            sendString += "END";
    //  sendString += "\n";
      // write the string to the server
          Particle.publish("FFTarray",sendString, PRIVATE);
    // start sampling again
    startSampling();
  }

void myHandler(const char *event, const char *data) {
    Serial.print("server response");
    Serial.println(data);
}



void triggerLeds(){
     if (audioMAP>=200){
        colorWipe(strip.Color(0, 255, 255), 250); // Blue
    }
    else if (audioMAP>=150){
        colorWipe(strip.Color(255, 255, 0), 250); //Yellow
    }
    else if (audioMAP>=100){
        colorWipe(strip.Color(255, 0, 255), 250); // Purple
    }
    else{
        colorWipe(strip.Color(0, 255, 40), 250);
    }
}
// EXTRA FUNCTIONS =====

void startSampling() {
  sampleCounter = 0;
  fillBuffer = true;
}

void stopSampling() {
  fillBuffer = false;
}

bool samplingIsDone() {
  return sampleCounter >= FFT_SIZE * 2;
}
