#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DS3231.h>
DS3231 myRTC;
DateTime myDT;
LiquidCrystal_I2C lcd(0x27, 20, 04);
//Constants
#define DHTPIN 5
#define DHTTYPE DHT22  // DHT 22  (AM2302)
#define numPins 7
#define on HIGH
#define off LOW
DHT dht(DHTPIN, DHTTYPE);  //// Initialize DHT sensor for normal 16mhz Arduino
//Variables
byte temperature = EEPROM.read(0);  // Set up  Temperature
byte humyditi = EEPROM.read(1);     // Set up humidity
bool state = EEPROM.read(2);        // True work like dehidratation False work for cooling
float hum;                          //Stores humidity value
float temp;                         //Stores temperature value
uint32_t timeStamp = EEPROM.get(3, timeStamp);
uint32_t compressorTs;
bool serialStatus = EEPROM.read(7);
byte fan_up_speed = EEPROM.read(8);
bool cooler_status = false;
bool dehumi_status = false;
bool humi_status = false;
bool heat_status = false;
bool circulation_status = false;
bool cooler_set = true;
bool hum_set = true;
byte Pin[7] = { 2, 8, 6, 10, 7, 9, 3}; //cooler, fan_fridge, lamp_fridge, fan_heat, humi, fan_up, fan_up_ctrl
byte downArrow[] = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04 };
byte upArrow[] = { 0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x00 };
void setup() {
  Wire.begin();
  Serial.begin(9600);
  /*----------------------------------------------------------------------------
    In order to synchronise your clock module, insert timetable values below !
    ----------------------------------------------------------------------------*/
  // myRTC.setClockMode(false);
  // myRTC.setHour(21);
  // myRTC.setMinute(8);
  // myRTC.setSecond(30);

  // myRTC.setDate(22);
  // myRTC.setMonth(1);
  // myRTC.setYear(23);
  lcd.begin();
  lcd.createChar(0, downArrow);
  lcd.createChar(1, upArrow);
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Meat Dehydrator");
  lcd.setCursor(1, 1);
  lcd.print("By Martin Atanasov");
  lcd.setCursor(4, 2);
  lcd.print("Ver. 1.3.2");
  for (byte i = 0; i < numPins; i++) {  //for each pin
    pinMode(Pin[i], OUTPUT);
    digitalWrite(Pin[i], off);
  }
  myDT = RTClib::now();
  dht.begin();
  delay(3000);
  lcd.clear();
  compressorTs = (myDT.unixtime() + 90);
}
void loop() {
  myDT = RTClib::now();
  uint32_t ReadDht;
  if (myDT.unixtime() >= ReadDht){
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    ReadDht = myDT.unixtime() + 2;
  }
  readSerial();  
  tempCtrl();
  humCtrl();
  fanUpCtrl();
  serialPrint();
  lcdPrint();
  delay(700);
}
void tempCtrl() {
  if (cooler_set) {
    if (temp > (temperature + 0.9)) {
      hum_set = false;
      compressor(on);
      cooler_status = true;
    } else if (temp <= temperature) {
      compressor(off);
      cooler_status = false;
      hum_set = true;
    }
  }
  if (temp <= (temperature - 0.6)) {
    digitalWrite(Pin[3], on);
    heat_status = true;
  } else if (temp >= temperature) {
    digitalWrite(Pin[3], off);
    heat_status = false;
  }
}
void humCtrl() {
  if (hum_set) {
    if (hum >= (humyditi + 4.9) && state) {
      compressor(on);
      cooler_set = false;
      dehumi_status = true;
    } else if (hum <= humyditi) {
      compressor(off);
      cooler_set = true;
      dehumi_status = false;
    }
  }
  if (hum <= (humyditi - 4.9) && state) {
    digitalWrite(Pin[4], on);
    humi_status = true;
  } else {
    digitalWrite(Pin[4], off);
    humi_status = false;
  }
}
void fanUpCtrl() {
  bool chk;
  switch (myDT.minute()) {
    case 0:
      chk = true;
      break;
    case 5:
      chk = true;
      break;
    case 10:
      chk = true;
      break;
    case 15:
      chk = true;
      break;
    case 20:
      chk = true;
    case 25:
      chk = true;
      break;
    case 30:
      chk = true;
      break;
    case 35:
      chk = true;
      break;
    case 40:
      chk = true;
      break;
    case 45:
      chk = true;
      break;
    case 50:
      chk = true;
      break;
    case 55:
      chk = true;
      break;
    default:
      chk = false;
  }
  if (chk && state) {
    analogWrite(Pin[6], fan_up_speed);
    digitalWrite(Pin[5], on);
    circulation_status = true;
  } else {
    analogWrite(Pin[6], 0);
    digitalWrite(Pin[5], off);
    circulation_status = false;
  }
}
void serialPrint() {
  if (serialStatus) {
    Serial.print("cooler:");
    Serial.print(cooler_status);
    Serial.print(" dehum:");
    Serial.print(dehumi_status);
    Serial.print(" heat:");
    Serial.print(heat_status);
    Serial.print(" humy:");
    Serial.print(humi_status);
    Serial.print(" circ:");
    Serial.print(circulation_status);
    Serial.print(" state:");
    Serial.print(state);
    Serial.print(" humStat:");
    Serial.print(hum_set);
    Serial.print(" CoolerStat:");
    Serial.print(cooler_set);
    Serial.print(" Fan Speed:");
    Serial.print(fan_up_speed);
    Serial.print(" T:");
    Serial.print(temperature);
    Serial.print(" H:");
    Serial.print(humyditi);
    Serial.print(" TS:");
    Serial.println(timeStamp);
    Serial.print("Hum: ");
    Serial.print(hum);
    Serial.print("    Temp: ");
    Serial.println(temp);
  } else {
    Serial.println("Enter 5");
  }
}
void lcdPrint() {
  lcd.cursor_off();  
  //lcd.clear();
  lcd.setCursor(0, 0);
  printDigitsLcd(myDT.day());
  lcd.print("/");
  printDigitsLcd(myDT.month());
  lcd.print("/");
  lcd.print(myDT.year());
  lcd.setCursor(15, 0);
  printDigitsLcd(myDT.hour());
  lcd.print(":");
  printDigitsLcd(myDT.minute());
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temp);
  if (cooler_status) {
    lcd.write(byte(0));
    lcd.print("  ");    
  } else if (heat_status) {
    lcd.write(byte(1));
  } else {
    lcd.print("  ");
  }
  lcd.setCursor(12, 1);
  lcd.print("H:");
  lcd.print(hum);
  if (dehumi_status) {
    lcd.write(byte(0));
  } else if (humi_status) {
    lcd.write(byte(1));
  } else {
    lcd.print("  ");
  }
  lcd.setCursor(0, 2);
  lcd.print("Mode:");
  if (state) {
    lcd.print("Dehydr.");
  } else {
    lcd.print("Cooler ");
  }
  lcd.setCursor(13, 2);
  lcd.print("Day:");
  printDigitsLcd(timeReturn());
  lcd.setCursor(0, 3);
  // printDigitsLcd(cooler_status);
  // printDigitsLcd(dehumi_status);
  // printDigitsLcd(heat_status);
  // printDigitsLcd(humi_status);
  // printDigitsLcd(circulation_status);
}
void printDigitsLcd(byte digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}
byte timeReturn() {
  byte dayResult = ((myDT.unixtime() - timeStamp) / 86400);
  return dayResult;
}
void compressor(byte i){
  bool k;
  if (myDT.unixtime() >= compressorTs){
    if (i == HIGH){
          digitalWrite(Pin[0], on);
          digitalWrite(Pin[1], on);
          k = true;
    }
    else if (i == LOW){
      digitalWrite(Pin[0], off);
      digitalWrite(Pin[1], off);
      if ( k == true ){
      compressorTs = (myDT.unixtime() +90);
      k= false;
      }
    }
  } 
}
void readSerial() {
  String error = "Wrong Input\n";
  String done = "DONE\n";
  String enaDis[2] = {"Enable\n", "Disable\n"};
  if (Serial.available() > 0) {
    byte a = Serial.parseInt();
    if (a == 1) {
      Serial.print("Temp: ");
      while (!Serial.available()) {}
      EEPROM.update(0, Serial.parseInt());
      temperature = EEPROM.read(0);
      Serial.println(temperature);
      Serial.println(done);
    } else if (a == 2) {
      Serial.print("Hum: ");
      while (!Serial.available()) {}
      EEPROM.update(1, Serial.parseInt());
      humyditi = EEPROM.read(1);
      Serial.println(humyditi);
      Serial.println(done);
    } else if (a == 3) {
      Serial.print("Mode: ");
      while (!Serial.available()) {}
      EEPROM.update(2, Serial.parseInt());
      state = EEPROM.read(2);
      Serial.println(state);
      Serial.println(done);
    } else if (a == 4) {
      Serial.print("Reset Day?: ");
      while (!Serial.available()) {}
      if (Serial.parseInt()) {
        EEPROM.put(3, myDT.unixtime());
        EEPROM.get(3, timeStamp);
        Serial.println(done);
      } else {
        Serial.println(error);
      }
    } else if (a == 5) {
      Serial.print("Serial: ");
      while (!Serial.available()) {}
      EEPROM.update(7, Serial.parseInt());
      serialStatus = EEPROM.read(7);
      if (serialStatus == 0)
        Serial.println(enaDis[1]);
      else if (serialStatus == 1) {
        Serial.println(enaDis[0]);
      } else {
        Serial.println(error);
      }
    } else if (a == 6) {
      Serial.print("Fan Speed: ");
      while (!Serial.available()) {}
      EEPROM.update(8, Serial.parseInt());
      fan_up_speed = EEPROM.read(8);
      Serial.print(fan_up_speed);
      Serial.println(done);
    } else {
      Serial.println(error);
    }
  }
}