/*
Lcd circuit:
 * LCD K pin to GND
 * LCD A pin to 330 ohm resistor to digital pin 6
 * LCD D7 pin to (digital) analog pin A1
 * LCD D6 pin to (digital) analog pin A2
 * LCD D5 pin to digital pin 10
 * LCD D4 pin to digital pin 9
 * LCD D3 pin
 * LCD D2 pin
 * LCD D1 pin
 * LCD D0 pin
 * LCD E pin to digital pin 8
 * LCD RW pin to GND
 * LCD RS pin to digital pin 7
 * LCD V0 pin to 10k pot
 * LCD VDD pin to (digital) analog pin A3
 * LCD VSS pin to GND
*/

/*
SD-card circuit:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
*/

#include <arduino.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
//#include <dht_nonblocking.h>
//#include <DHT.h>


#include "customLcdChars.h"

//#define DHT_SENSOR_TYPE DHT_TYPE_11
//#define DHTTYPE DHT22
//#define DHTPIN 6

//Global constants
const char filename[] = "datalog.txt";
const int SOIL_MOISTURE_CALIBRATION_AIR = 530; // old 615
const int SOIL_MOISTURE_CALIBRATION_WATER = 213;
const int musicDry[] = {440, 550, 330};
const int musicLength = 3;

//Analogue pins
const int MOISTURE_PIN =  0;
const int THERMISTOR_PIN =      A4;
const int LIGHT_PIN =     A5;
//Digital pins
const int BUTTON1_PIN =   2;
const int BUTTON2_PIN =   3;
const int SDCS =          4;
const int BUZZER_PIN =    5;
//static const int DHT_SENSOR_PIN = 6;

//LCD pins
const int LCD_BRIGHTNESS = 6; //A
const int D7 = A1;
const int D6 = A2;
const int D5 = 10;
const int D4 = 9;
const int E = 8;
const int RS = 7;
const int LCD_ENABLE = A3; //VDD

//Global variables for the state machine
byte soilMoistureMeasureState = 0;
byte temperatureMeasureState = 0;
byte lightMeasureState = 0;
byte sensorDataReady = 0;
byte buzzerState = 0;
bool sdCardState = false;
bool callLcdUpdate = false;
enum lcdState {         //TODO: change to only containing one state for every menu item. Add another state if changing value
  FACE,
  DATA,
  MENU_SELECT_LOWER,
  MENU_SELECT_HIGHER,
  MENU_CHANGE_LOWER,
  MENU_CHANGE_HIGHER,
  OFF
};
enum faceState {
  WET,
  HAPPY,
  THIRSTY
  // BATTERY_LOW
};
enum buttonState {
  PRESSED,
  NOT_PRESSED
};
enum lcdState lcdState = FACE;
enum buttonState buttonState = NOT_PRESSED;
enum faceState faceState = HAPPY;
enum faceState oldFaceState = faceState;

//Other global variables
const int sensorMeasureDelay = 2000;      //delay between readings 15min 900000
const int sesnorMeasureWaitingTime = 200;   //delay between readings when calculating average
const int buzzerWaitingTime = 1000;         //time between notes
const int buzzerWaitingDelay = 3000;        //time between soundtrack

int soilMoistureReadingCount = 0;
int soilMoistureReading = 0;
int soilMoistureReadingAverage = 0;
int soilMoistureCalibrated = 0;
int soilMoisturePercentage = 0;
int soilMoistureTresholdWet = 50;
int soilMoistureTresholdDry = 30;

int temperatureReadingCount = 0;
int temperatureReading = 0;
int temperatureReadingAverage = 0;
float temperature = 0; 

int lightReadingCount = 0;
int lightReading = 0;
int lightReadingAverage = 0;

unsigned long soilMoistureMeasureTime = 0;
unsigned long temperatureMeasureTime = 0;
unsigned long lightMeasureTime = 0;
unsigned long buzzerTime = 0;

int musicIndex = 0;

// const int lcdUpdateWaitingTime = 5000;     //delay between lcd updates 
// unsigned long lcdUpdateTime = 0;

// const int serialUpdateWaitingTime = 10000;   //delay between serial updates 10sec
// unsigned long serialUpdateTime = 0;


//TODO: change these
char temperatureChar[6];
char strBuf[120];
char lcdTop[16];
char lcdBottom[16];

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

//DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
//DHT dht(DHTPIN, DHTTYPE);


void button1Fxn(){    //näkymänvaihtonappi
  Serial.println("Button1 pressed!");
  switch (lcdState)
  {
  case DATA:
  case FACE:
    lcdState = MENU_SELECT_LOWER;
    break;
  
  case MENU_SELECT_LOWER:
    lcdState = MENU_SELECT_HIGHER;
    break;
  
  case MENU_SELECT_HIGHER:
    lcdState = DATA;
    break;

  case MENU_CHANGE_LOWER:
    soilMoistureTresholdDry += 5;
    if (soilMoistureTresholdDry > 100){
      soilMoistureTresholdDry -= 100;
    }
    break;
  
  case MENU_CHANGE_HIGHER:
    soilMoistureTresholdWet += 5;
    if (soilMoistureTresholdWet > 100){
      soilMoistureTresholdWet -= 100;
    }
    break;

  default:
    Serial.println("invalid menustate!");
    break;
  }
  callLcdUpdate = true;
}


void button2Fxn(){
  switch (lcdState)
  {
  case DATA:
    lcdState = FACE;
    break;

  case FACE:
    lcdState = DATA;
    break;
  
  case MENU_SELECT_LOWER:
    lcdState = MENU_CHANGE_LOWER;
    break;
  
  case MENU_SELECT_HIGHER:
    lcdState = MENU_CHANGE_HIGHER;
    break;

  case MENU_CHANGE_LOWER:
    lcdState = DATA;
    break;
  
  case MENU_CHANGE_HIGHER:
    lcdState = DATA;
    break;

  default:
    Serial.println("invalid menustate!");
    break;
  }
  callLcdUpdate = true;
}

bool writeToFile(String dataString){
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println("Data sent to SD-card!");
    return true;
  } else {
    sprintf(strBuf, "error opening %s", filename);
    Serial.println(strBuf);
    return false;
  }
}


void measureSoilMoisture(){
  switch(soilMoistureMeasureState)
  {
  case 0:
    soilMoistureReadingCount = 0;
    soilMoistureReadingAverage = 0;
    soilMoistureMeasureTime = millis();
    soilMoistureMeasureState++;
    break;
  case 1: //read data
    if((soilMoistureReadingCount < 10) && (millis() - soilMoistureMeasureTime >= sesnorMeasureWaitingTime)){
      soilMoistureMeasureTime = millis();
      soilMoistureReading = analogRead(MOISTURE_PIN);
      soilMoistureReadingAverage += soilMoistureReading;
      soilMoistureReadingCount++;
    } 
    if(soilMoistureReadingCount >= 10) {
      soilMoistureMeasureState++;        
    }
    break;
  case 2: //calculate soil moisture percentage
    soilMoistureReadingAverage = soilMoistureReadingAverage / soilMoistureReadingCount;
    soilMoistureCalibrated = map(soilMoistureReadingAverage, SOIL_MOISTURE_CALIBRATION_WATER, SOIL_MOISTURE_CALIBRATION_AIR, 1320, 3173);    //calibrate new value with map function
    soilMoisturePercentage = (178147020.5 - 52879.727 * soilMoistureCalibrated) / (1 - 428.814 * soilMoistureCalibrated + 0.9414 * pow(soilMoistureCalibrated, 2)); //Fitting function to calculate %-value
    if (soilMoisturePercentage > 100) {
      soilMoisturePercentage = 100; 
    }
    if (soilMoisturePercentage < 0) {
      soilMoisturePercentage = 0; 
    }

    //Set face state
    if (soilMoisturePercentage < soilMoistureTresholdDry){
      faceState = THIRSTY;
    } else if (soilMoisturePercentage < soilMoistureTresholdWet ){
      faceState = HAPPY;
    } else {
      faceState = WET;
    }

    soilMoistureMeasureState++;
    soilMoistureMeasureTime = millis();   
    break;
  case 3:
    break;
  case 4:
    if (millis() - soilMoistureMeasureTime >= sensorMeasureDelay){
      soilMoistureMeasureState = 0; //mark as done
    }
    break;
  }
}

void measureLight(){
  switch (lightMeasureState)
  {
  case 0:
    lightReadingCount = 0;
    lightReadingAverage = 0;
    lightMeasureTime = millis();
    lightMeasureState++;
    break;
  case 1:
    if((lightReadingCount < 10) && (millis() - lightMeasureTime >= sesnorMeasureWaitingTime)){
        lightMeasureTime = millis();
        lightReading = analogRead(LIGHT_PIN);
        lightReadingAverage += lightReading;
        lightReadingCount++;
    }
    if(lightReadingCount >= 10){
      lightMeasureState++;        
    }
    break;
  case 2:
    lightReadingAverage = lightReadingAverage / lightReadingCount;

    lightMeasureState++; 
    lightMeasureTime = millis();    
    break;
  case 3:
    break;
  case 4:
    if (millis() - lightMeasureTime >= sensorMeasureDelay){
      lightMeasureState = 0; //mark as done
    }
    break;

  }
}


void measureTemperature(){
  switch (temperatureMeasureState)
  {
  case 0:
    temperatureReadingCount = 0;
    temperatureReadingAverage = 0;
    temperatureMeasureTime = millis();
    temperatureMeasureState++;
    break;
  case 1:
    if((temperatureReadingCount < 10) && (millis() - temperatureMeasureTime >= sesnorMeasureWaitingTime)){
        temperatureMeasureTime = millis();
        temperatureReading = analogRead(THERMISTOR_PIN);
        temperatureReadingAverage += temperatureReading;
        temperatureReadingCount++;
    } 
    if(temperatureReadingCount >= 10){
      temperatureMeasureState++;        
    }
    break;
  {
  case 2:
    temperatureReadingAverage = temperatureReadingAverage / temperatureReadingCount;
    float rThermistor = 10000 * (float(1024)/temperatureReadingAverage - 1.0);  // Thermistor resistance
    float tKelvin = 1.0/(0.001022197 + 0.000253179 * log(rThermistor) - 0.0000000000587992 * (pow(log(rThermistor),3)));
    temperature = tKelvin - 273.15;                // Convert Kelvin to Celsius
    dtostrf(temperature, 3, 1, temperatureChar);
    //double tempK = log(10000.0 * ((1024.0 / tempReading - 1.0)));
    //tempK = 1 / (0.001022197 + (0.000253179 - (0.0000000000587992 * tempK * tempK )) * tempK );       //  Temp Kelvin
    //temperature = tempK - 273.15;            // Convert Kelvin to Celcius

    temperatureMeasureState++; 
    temperatureMeasureTime = millis();
    break;
  }
  case 3:
    break;
  case 4:
    if (millis() - temperatureMeasureTime >= sensorMeasureDelay){
      temperatureMeasureState = 0; //mark as done
    }
    break;

  }
}

void readSensors(){
  measureSoilMoisture();
  measureLight();
  measureTemperature();
  if (lightMeasureState == 3 && soilMoistureMeasureState == 3 && temperatureMeasureState == 3){
    sensorDataReady = 1;
    soilMoistureMeasureState = 4;
    temperatureMeasureState = 4;
    lightMeasureState = 4;
    soilMoistureMeasureTime = millis();
    temperatureMeasureTime = millis();
    lightMeasureTime = millis();
    Serial.println("sensorDataReady!");
  }
}

void sendDataToSD(){
  if (sdCardState){
  //Moisture_raw, moisture percent, temperature, light
  sprintf(strBuf, "%d, %d, %s, %d, %lu", soilMoistureReadingAverage, soilMoisturePercentage, temperatureChar, lightReading, (millis() / 60000));
  writeToFile(strBuf);
  }
}

void printToSerial(){
  // if((millis() - serialUpdateTime) >= serialUpdateWaitingTime){
    //send to serial
    sprintf(strBuf, "Moisture %%: %d%%, Moisture raw avg: %d, Moisture raw: %d, Temperature: %s, Light raw: %d, faceState: %d",
      soilMoisturePercentage,
      soilMoistureReadingAverage,
      analogRead(MOISTURE_PIN),
      temperatureChar,
      lightReadingAverage,
      faceState
      );     
    // serialUpdateTime = millis();
    Serial.println(strBuf);
  // }
}

void selectFace(){
  if (oldFaceState != faceState){
    switch (faceState)
    {
    case HAPPY:
      lcd.createChar(2, faceBottomLeftHappy);
      lcd.createChar(3, faceBottomMiddleHappy);
      lcd.createChar(4, faceBottomRightHappy);
      lcd.createChar(5, faceTopLeftHappy);
      lcd.createChar(6, faceTopMiddleHappy);
      lcd.createChar(7, faceTopRightHappy);
      break;

    case THIRSTY:
      lcd.createChar(2, faceBottomLeftThirsty);
      lcd.createChar(3, faceBottomMiddleThirsty);
      lcd.createChar(4, faceBottomRightThirsty);
      lcd.createChar(5, faceTopLeftThirsty);
      lcd.createChar(6, faceTopMiddleThirsty);
      lcd.createChar(7, faceTopRightThirsty);
      break;

    case WET:
      lcd.createChar(2, faceBottomLeftWet);
      lcd.createChar(3, faceBottomMiddleWet);
      lcd.createChar(4, faceBottomRightWet);
      lcd.createChar(5, faceTopLeftWet);
      lcd.createChar(6, faceTopMiddleWet);
      lcd.createChar(7, faceTopRightWet);
      break;
    
    // case BATTERY_LOW:
    //   lcd.createChar(2, faceBottomLeftBatteryLow);
    //   lcd.createChar(3, faceBottomMiddleBatteryLow);
    //   lcd.createChar(4, faceBottomRightBatteryLow);
    //   lcd.createChar(5, faceTopLeftBatteryLow);
    //   lcd.createChar(6, faceTopMiddleBatteryLow);
    //   lcd.createChar(7, faceTopRightBatteryLow);
    //   break;
    }
  }
  oldFaceState = faceState;
}

void updateLcd(){  
  if((sensorDataReady == 1) || callLcdUpdate){
    switch (lcdState)
    {
    case DATA:
      lcd.clear(); 
      
      sprintf(lcdTop, "%3d%% RAW: %d", soilMoisturePercentage, soilMoistureReading);
      lcd.setCursor(0,0);
      lcd.write(byte(0)); 
      lcd.setCursor(2,0);
      lcd.print(lcdTop);
      
      lcd.setCursor(0,1);
      lcd.write(byte(1));
      lcd.setCursor(2,1);
      sprintf(lcdBottom, "%s%cC : %d", temperatureChar, (char)223, lightReadingAverage);
      lcd.print(lcdBottom);
      break;

    case FACE:
      lcd.clear();

      sprintf(lcdTop, "%3d%%", soilMoisturePercentage);
      lcd.setCursor(0,0);
      lcd.write(byte(0)); 
      lcd.setCursor(2,0);
      lcd.print(lcdTop);
      Serial.println(lcdTop);

      lcd.setCursor(0,1);
      lcd.write(byte(1));
      lcd.setCursor(2,1);
      sprintf(lcdBottom, "%s%cC", temperatureChar, (char)223);
      lcd.print(lcdBottom);

      //Print the face 
      selectFace();
      lcd.setCursor(13,1);
      lcd.write(byte(2));  
      lcd.setCursor(14,1);
      lcd.write(byte(3));  
      lcd.setCursor(15,1);
      lcd.write(byte(4));  
      lcd.setCursor(13,0);
      lcd.write(byte(5));  
      lcd.setCursor(14,0);
      lcd.write(byte(6));  
      lcd.setCursor(15,0);
      lcd.write(byte(7));  
      break;
    
    case MENU_SELECT_LOWER:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("MENU: lower");
      lcd.setCursor(0,1);
      lcd.write("moist treshold");
      break;
    
    case MENU_SELECT_HIGHER:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("MENU: higher");
      lcd.setCursor(0,1);
      lcd.write("moist treshold");
      break;

    case MENU_CHANGE_LOWER:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("lower moisture treshold");
      sprintf(lcdTop, "%d%%", soilMoistureTresholdDry);
      lcd.setCursor(0,1);
      lcd.write(byte(0)); 
      lcd.setCursor(2,1);
      lcd.print(lcdTop);
      break;
    
    case MENU_CHANGE_HIGHER:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("higher moisture treshold");
      sprintf(lcdTop, "%d%%", soilMoistureTresholdWet);
      lcd.setCursor(0,1);
      lcd.write(byte(0)); 
      lcd.setCursor(2,1);
      lcd.print(lcdTop);
      break;

    default:
      Serial.println("invalid menustate!");
      break;
    }
    // lcdUpdateTime = millis();
    Serial.println("Lcd updated!");
    callLcdUpdate = false;
  }
}

void buzzer() {
  switch (buzzerState)
  {
  case 0: //waiting
    if ((faceState == THIRSTY) && (millis() - buzzerTime > buzzerWaitingDelay)){
      buzzerTime = millis();
      musicIndex = 0;
      buzzerState++;
    } 
    break;

  case 1: //playing
    if (millis() - buzzerTime > buzzerWaitingTime){
      tone(BUZZER_PIN, musicDry[musicIndex]);
      musicIndex++;
      if (musicIndex == musicLength + 1){
        noTone(BUZZER_PIN);
        buzzerState--;
      }
      buzzerTime = millis();
    }
    break;
  }
}

void intializeSdCard(){
  Serial.println("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(SDCS)) {
    Serial.println("Card failed, or not present");
    sdCardState = false;
  }
  sdCardState = true;
  Serial.println("card initialized.");
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), button1Fxn, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), button2Fxn, RISING);
  pinMode(LCD_BRIGHTNESS, OUTPUT);
  pinMode(LCD_ENABLE, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D6, OUTPUT);
  analogWrite(LCD_BRIGHTNESS, 255);  
  digitalWrite(LCD_ENABLE, HIGH);

  delay(200);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  intializeSdCard();

  // writeToFile("Moisture_raw, moisture percent, temperature, light, timeFromStart");
  
  lcd.begin(16, 2); //set up the LCD's number of columns and rows
  
  //Custom characters for lcd
   lcd.createChar(0, drop);
   lcd.createChar(1, temp);
   lcd.createChar(2, faceBottomLeftHappy);
   lcd.createChar(3, faceBottomMiddleHappy);
   lcd.createChar(4, faceBottomRightHappy);
   lcd.createChar(5, faceTopLeftHappy);
   lcd.createChar(6, faceTopMiddleHappy);
   lcd.createChar(7, faceTopRightHappy);

  //  lightMeasureState = 1;
  //  temperatureMeasureState = 1;
  //  soilMoistureMeasureState = 1;
}

void writeToSD(char dataString[]){
  if (sdCardState){
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open(filename, FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
  }
}

void loop() {
  readSensors();
  // buzzer();
  //TODO: one send data funktion for serial and sd?
  if (sensorDataReady == 1){
    //sendDataToSD();
    printToSerial();
  }
  updateLcd();
  sensorDataReady = 0;

// make a string for assembling the data to log:
  char dataString[] = "12345";
  if (sdCardState){
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
  }
}
