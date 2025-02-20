#include "ACS712.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Define the SDA and SCL pins manually
#define OLED_SDA 27
#define OLED_SCL 28 // Update accordingly

const int powerSwPin 26;

float readings = 0;
const int sensorIn = 34;  // pin where the OUT pin from sensor is connected on Arduino
int mVperAmp = 185;       // this the 5A version of the ACS712 -use 100 for 20A Module and 66 for 30A Module
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(22, OUTPUT);
  pinMode(powerSwPin, OUTPUT);
  pinMode(34, INPUT);
  Serial.println("Starting...");
  // Manually set the I2C pins
  Wire.begin(OLED_SDA, OLED_SCL);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  // Set brightness (for SSD1306, contrast mimics brightness)
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0xFF);  // Max contrast

  // Fill the screen with white pixels
  display.clearDisplay();
  display.fillScreen(SSD1306_WHITE);
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(22, HIGH);
  digitalWrite(powerSwPin, HIGH);
  delay(100);


  Serial.print("High: ");
  readData();


  delay(5000);
  digitalWrite(22, LOW);
  digitalWrite(powerSwPin, LOW);


  Serial.print("low: ");
  readData();

  delay(5000);
}

void readData() {
  Voltage = getVPP();
  VRMS = (Voltage / 2.0) * 0.707;              //root 2 is 0.707
  AmpsRMS = ((VRMS * 1000) / mVperAmp) - 0.4;  //0.3 is the error I got for my sensor
  Serial.print(AmpsRMS);
  Serial.print("Amps RMS  ---  ");
  Watt = (AmpsRMS * 3.3);
  // note: 1.2 is my own empirically established calibration factor
  // as the voltage measured at D34 depends on the length of the OUT-to-D34 wire
  // 240 is the main AC power voltage – this parameter changes locally
  Serial.print(Watt);
  Serial.println(" Watts");
}
// ***** function calls ******
float getVPP() {
  float result;
  int readValue;        // value read from the sensor
  int maxValue = 0;     // store max value here
  int minValue = 4096;  // store min value here ESP32 ADC resolution

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000)  //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);

    // see if you have a new maxValue
    if (readValue > maxValue) {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue) {
      /*record the minimum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 3.3) / 4096.0;  //ESP32 ADC resolution 4096

  return result;
}