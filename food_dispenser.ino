#include <Arduino.h>
#include "HX711.h"
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>
Servo myservo;
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2C lcd1(0x26, 16, 2);

//Communication
#define rxPin 10
#define txPin 11
SoftwareSerial megaSerial =  SoftwareSerial(rxPin, txPin);
//DHT11
const int DHTPIN = 8;
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
int pos = 0;
HX711 scale;

unsigned long timedelay;
unsigned long timedelay1;
void setup() {
  Serial.begin(19200);
  megaSerial.begin(9600);
  dht.begin();
  lcd.begin();
  lcd1.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd1.backlight();
  lcd.print("Initialized...");
  lcd1.print("Initialized...");
  Serial.println("HX711 Demo");
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());  // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));  // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
  // by the SCALE parameter (not set yet)

  scale.set_scale(-365.897);
  //scale.set_scale(-471.497);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();  // reset the scale to

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());  // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));  // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight, divided
  // by the SCALE parameter set with set_scale

  Serial.println("Readings:");
  timedelay = millis();
  timedelay1 = millis();
  lcd.clear();
  lcd1.clear();
  myservo.attach(9);

}
int i = 0;
int thresholdWeight = 100;
int thresholdTimer = 0;
float tempW = 0;
char c;
String dataIn;
String tempWeight = "";
String tempInterval = "";
int isUpdate = 0;
String dataArray[10];
int dataIdx = 0;
String correctdataIn = "";
int count = 0;
int previousCount = 0;
int idx, idx1;
float prev_w=0;
void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  lcd1.setCursor(1, 0);
  lcd1.print("Nhiet do: ");
  lcd1.print(round(t));
  lcd1.print("oC");
  lcd1.setCursor(1, 1);
  lcd1.print("Do am: ");
  lcd1.print(round(h));
  lcd1.print("%");

  //Receive data from ESP32
  while (megaSerial.available()) {
    c = megaSerial.read();
    if (c != '\n') {
      dataIn += c;
    }
    else {
      break;
    }
  }
  if (c == '\n') {
    Serial.println(dataIn);
    if (dataIdx < 10 && isUpdate) {
      Serial.println("if1:" + dataIdx);
      dataArray[dataIdx] = dataIn;
      dataIdx++;
    }
    else if (dataIdx >= 10 && isUpdate) {
      Serial.println("if2:" + dataIdx);
      for (int j = 0; j < 10; j++) {
        count = 0;
        correctdataIn = dataArray[j];
        for (int k = j; k < 10; k++) {
          if (strcmp(dataArray[j].c_str(), dataArray[k].c_str()) == 0) {
            count++;
          }
        }
        if (previousCount < count) {
          previousCount = count;
          correctdataIn = dataArray[j];
        }
      }
      idx = correctdataIn.indexOf(",");
      //    isUpdate=correctdataIn.substring(0,idx).toInt();
      idx1 = correctdataIn.indexOf(",", idx + 1);
      tempWeight = correctdataIn.substring(idx + 1, idx1);
      thresholdWeight = tempWeight.toInt();
      tempInterval = correctdataIn.substring(idx1 + 1);
      thresholdTimer = tempInterval.toInt();
      isUpdate = 0;
      dataIdx = 0;
    }
    if (dataIn.substring(0, 1).toInt() == 1) {
      isUpdate = 1;
      dataIdx = 0;
    }
    
    //    megaSerial.print(String(int(round(t)))+","+String(int(round(h)))+"\n");
    c = 0;
    dataIn = "";
  }
  //


  if (i == 0) {
    for (int j = 0; j < 3; j++) {
      lcd.setCursor(1, 0);
      lcd.print("Get food:");
      lcd.setCursor(1, 1);
      tempW = int(round(scale.get_units(5)));
      lcd.print(tempW);
      delay(1000);
    }
    lcd.setCursor(1, 0);
    lcd.print("Current W:");
    i++;
  }
  else {
    lcd.setCursor(1, 0);
    lcd.print("Current W:");
    lcd.setCursor(1, 1);
    tempW = float(round(scale.get_units(5)));
    if(tempW<0){
      tempW=0;
    }
    lcd.print(tempW);
  }
  if(isUpdate==0&&strcmp(correctdataIn.c_str(),"")!=0){
      Serial.println(thresholdWeight);
    Serial.println("---");
    Serial.println(thresholdTimer);
    if((unsigned long)(millis()-timedelay)>thresholdTimer*1000){
    //open
    Serial.println("Open");
    for(pos = 10; pos < 180; pos += 1){
  
          myservo.write(pos);
          delay(5);
      }
      timedelay1=millis();
       while(1){
      lcd.setCursor(1,1);
      tempW=round(scale.get_units(5));
      if(tempW<0){
      tempW=0;
      }
      lcd.print(tempW);
      if(int(tempW)+20-thresholdWeight>=0){
  
      break;
       }
       if((unsigned long)(millis()-timedelay1)>20000&&tempW-prev_w<=0){
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Warning!..");
        timedelay1=millis();
        break;
       }
       prev_w=tempW;
    }
      //close
      for(pos = 180; pos>=10; pos-=1) {
          myservo.write(pos);
          delay(5);
      }
  
    timedelay=millis();
    }
    }
  


}
