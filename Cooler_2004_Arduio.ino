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
#define DHTPIN 2
#define DHTTYPE DHT22  // DHT 22  (AM2302)
#define numPins 5
#define on HIGH
#define off LOW
DHT dht(DHTPIN, DHTTYPE);  //// Initialize DHT sensor for normal 16mhz Arduino
//Variables
int temperature = EEPROM.read(0);  // Set up  Temperature
byte humyditi = EEPROM.read(4);     // Set up humidity
bool state = EEPROM.read(5);        // True work like dehidratation False work for cooling
float hum;                          //Stores humidity value
float temp;                         //Stores temperature value
uint32_t timeStamp = EEPROM.get(6, timeStamp);
uint32_t compressorTs;
bool serialStatus = EEPROM.read(10);
byte fan_up_speed = EEPROM.read(11);
byte serialCheck = 0;
bool cooler_status = false;
bool dehumi_status = false;
bool humi_status = false;
bool heat_status = false;
bool circulation_status = false;
bool cooler_set = true;
bool hum_set = true;
bool fanSpeed = false;
bool kompressor = true;
bool fanOnOff = EEPROM.read(12);
bool compressorWait = true ;
const byte Pin[5] = { 3, 4, 5, 6, 7 };  //cooler, Heating, Humyfider, Circulation, fan_up_ctrl
byte downArrow[] = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04 };
byte upArrow[] = { 0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x00 };
byte heatSymbol [] = { 0x04, 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x1F };
byte coolerSymbol [] = { 0x04, 0x15, 0x0E, 0x04, 0x0E, 0x15, 0x04, 0x00 };
byte dehumySymbol [] = { 0x00, 0x00, 0x04, 0x0E, 0x17, 0x17, 0x17, 0x0E };
byte humySymbol [] = { 0x00, 0x00, 0x04, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E };
byte circSymbol [] = { 0x10, 0x11, 0x0A, 0x04, 0x10, 0x11, 0x0A, 0x04 };
void setup() {
  Wire.begin();
  Serial.begin(115200);
  /*----------------------------------------------------------------------------
    In order to synchronise your clock module, insert timetable values below !
    ----------------------------------------------------------------------------*/
  // myRTC.setClockMode(false);
  // myRTC.setHour(15);
  // myRTC.setMinute(9);
  // myRTC.setSecond(30);

  // myRTC.setDate(19);
  // myRTC.setMonth(4);
  // myRTC.setYear(23);
  lcd.begin();
  lcd.createChar(0, downArrow);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, heatSymbol);
  lcd.createChar(3, coolerSymbol);
  lcd.createChar(4, dehumySymbol);
  lcd.createChar(5, humySymbol);
  lcd.createChar(6, circSymbol);
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
  compressorTs = (myDT.unixtime() + 120);
  
}
void loop() {
  myDT = RTClib::now();
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  delay(2000);
  readSerial();
  tempCtrl();
  humCtrl();
  fanUpCtrl();
  serialPrint();
  lcdPrint();
}
void tempCtrl() {
  if (cooler_set) {
    if (temp > (temperature + 0.9)) {
      hum_set = false;
      compressor(on);
      cooler_status = true;
    } else if (temp <= (temperature + 0.5)) {
      compressor(off);
      cooler_status = false;
      hum_set = true;
    }
  }
  if (temp <= (temperature - 1)) {
    digitalWrite(Pin[1], on);
    heat_status = true;
  } else if (temp >= temperature -0.5) {
    digitalWrite(Pin[1], off);
    heat_status = false;
  }
}
void humCtrl() {
  if (hum_set) {
    if (hum >= (humyditi + 4.9) && state) {
      compressor(on);
      cooler_set = false;
      dehumi_status = true;
      fanSpeed = true;
      if (temp <= temperature ) {
        digitalWrite(Pin[1], on);
      } else if (temp >= (temperature + 0.9)) {
        digitalWrite(Pin[1], off);
      }
    } else if (hum <= (humyditi + 4.4)) {
      compressor(off);
      cooler_set = true;
      dehumi_status = false;
      fanSpeed = false;
    }
  }
  if (hum <= (humyditi - 1) && state) {
    digitalWrite(Pin[2], on);
    humi_status = true;
  } else if (hum >= (humyditi - 0.5)) {
    
    digitalWrite(Pin[2], off);
    humi_status = false;
  }
}
void fanUpCtrl() {
  
  bool chk;
  analogWrite(Pin[4], fan_up_speed);
  digitalWrite(Pin[3], fanOnOff); // WARNING
 
   switch (myDT.hour()) {
    case 0:
      chk = true;
      break;
    case 6:
      chk = true;
      break;
    case 12:
      chk = true;
      break;
    case 18:
      chk = true;
      break;
    // case 20:
    //   chk = true;
    // case 25:
    //   chk = true;
    //   break;
    // case 30:
    //   chk = true;
    //   break;
    // case 35:
    //   chk = true;
    //   break;
    // case 40:
    //   chk = true;
    //   break;
    // case 45:
    //   chk = true;
    //   break;
    // case 50:
    //   chk = true;
    //   break;
    // case 55:
    //   chk = true;
    //   break;
    default:
      chk = false;
  }
  if (chk && state && (myDT.minute() <=5)) {
    fanOnOff =EEPROM.read(12);
  } else {
    fanOnOff =false;
  }
  

}
void serialPrint() {
  if (serialStatus) {
    Serial.print("Cooler:");
    if (cooler_status == true) {

      Serial.print("ON");
      if (compressorWait == false){
        Serial.print("!,");
      }
      else{
        Serial.print(",");
      }
    } else {
      Serial.print("OFF,");
    }
    Serial.print(" Dehum:");
    if (dehumi_status == true) {
      Serial.print("ON,");
    } else {
      Serial.print("OFF,");
    }
    Serial.print(" Heat:");
    if (heat_status == true) {
      Serial.print("ON,");
    } else {
      Serial.print("OFF,");
    }
    Serial.print(" Humy:");
    if (humi_status == true) {
      Serial.print("ON,");
    } else {
      Serial.print("OFF,");
    }
    Serial.print(" FanHS:");
    if (circulation_status == true) {
      Serial.print("ON,");
    } else {
      Serial.print("OFF,");
    }
    Serial.print(" Mode:");
    if (state == true) {
      Serial.print("Dehydrating,");
    } else {
      Serial.print("Cooler/Freeze,");
    }
    Serial.print(" Humy:");
    if (hum_set == true) {
      Serial.print("Enabled,");
    } else {
      Serial.print("Disabled,");
    }
    Serial.print(" Cooler:");
    if (cooler_set == true) {
      Serial.print("Enabled,");
    } else {
      Serial.print("Disabled,");
    }
    Serial.print(" Fan/Speed:");
    if (fanOnOff) {
      Serial.print("ON/");
      Serial.print(fan_up_speed);
      Serial.print(",");
    } else {
      Serial.print("OFF,");
    }
    Serial.print(" T:");
    Serial.print(temperature);
    Serial.print(",");
    Serial.print(" H:");
    Serial.print(humyditi);
    Serial.print(",");
    Serial.print(" Day/s:");
    Serial.println(timeReturn());
    Serial.print("Hum:");
    Serial.print(hum);
    Serial.print("  Temp:");
    Serial.println(temp);
  } else {
    if (serialCheck < 1){
    Serial.println("Serial Disabled, Enter 0 for MENU");
    serialCheck = 2;
    }
    
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
    if (compressorWait == false){
      lcd.print("!");
    }
    else {
      lcd.print(" ");
    }
    lcd.print("  ");
  } else if (heat_status) {
    lcd.write(byte(1));
  } else {
    lcd.print("   ");
  }
  lcd.setCursor(12, 1);
  lcd.print("H:");
  lcd.print(hum);
  if (dehumi_status) {
    lcd.write(byte(0));
  } else if (humi_status) {
    lcd.write(byte(1));
  } else {
    lcd.print("   ");
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
  lcd.print("                    ");
  if (cooler_status & compressorWait){
    lcd.setCursor(1, 3);
    lcd.write(byte(3));
  }
  if (dehumi_status){
    lcd.setCursor(3, 3);
    lcd.write(byte(4));
  }
  if (heat_status){
    lcd.setCursor(5, 3);
    lcd.write(byte(2));
  }
  if (humi_status){
    lcd.setCursor(7, 3);
    lcd.write(byte(5));    
  }
  if (fanOnOff){
    lcd.setCursor(9, 3);
    lcd.write(byte(6)); 
  }

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
void compressor(byte i) {
  if (myDT.unixtime() >= compressorTs) {
    if (i == HIGH) {
      digitalWrite(Pin[0], on);
      kompressor = true;
    } else if (i == LOW) {
      digitalWrite(Pin[0], off);
      if (kompressor == true) {
        compressorTs = (myDT.unixtime() + 120);
        delay(100);
        kompressor = false;
      }
    }
    compressorWait = true;
  } else {
    compressorWait = false;
  }
}
void readSerial() {
  String error = "Wrong Input\n";
  String done = "DONE\n";
  int pause = 3000;
  String enaDis[2] = { "Enable\n", "Disable\n" };

   if (Serial.available() && Serial.parseInt() == 0) {
     Serial.flush();
     Serial.read();
     //Serial.end();
    // Serial.begin(1152000);
     Serial.print("MENU: \n1- Temperature Selection\n2- Humidity Selection\n3- Mode\n4- Reset Count Days\n5- Serial (Enable-Disable)\n6- Fan Speed 1-255\n7- Circulation FAN (ON-OFF) \n\n");
    while (Serial.available()==0) {};
   
    if (Serial.available()) {
      while (!Serial.available());
    byte a = Serial.parseInt();
    if (a == 1) {
      Serial.print("Temp: ");
      while (!Serial.available()) {}
      EEPROM.update(0, Serial.parseInt());
      temperature = EEPROM.read(0);
      Serial.println(temperature);
      Serial.println(done);
      delay(pause);
    } else if (a == 2) {
      Serial.print("Hum: ");
      while (!Serial.available()) {}
      EEPROM.update(4, Serial.parseInt());
      humyditi = EEPROM.read(4);
      Serial.println(humyditi);
      Serial.println(done);
      delay(pause);
    } else if (a == 3) {
      Serial.print("Mode(0 - Cooler/Freeze, 1-Dehydration): ");
      while (!Serial.available()) {}
      EEPROM.update(5, Serial.parseInt());
      state = EEPROM.read(5);
      if (state == 1) {
        Serial.println(" Dehydration ");
      } else {
        Serial.println(" Cooler/Freeze ");
      }
      Serial.println(done);
      delay(pause);
    } else if (a == 4) {
      Serial.print("Reset Day?(0 - NO, 1-YES): ");
      while (!Serial.available()) {}
      if (Serial.parseInt()) {
        EEPROM.put(6, myDT.unixtime());
        EEPROM.get(6, timeStamp);
        Serial.print(" Day Counter Reset");
      } else {
        Serial.print("Day Counter Not Reset");
      }
      delay(pause);
    } else if (a == 5) {
      Serial.print("Serial(0 - OFF, 1-ON): ");
      while (!Serial.available()) {}
      EEPROM.update(10, Serial.parseInt());
      serialStatus = EEPROM.read(10);
      if (serialStatus == 0)
        Serial.println(enaDis[1]);
      else if (serialStatus == 1) {
        Serial.println(enaDis[0]);
      } else {
        Serial.println(error);
      }
    } else if (a == 6) {
      Serial.print("Fan Speed (1-255): ");
      while (!Serial.available()) {}
      EEPROM.update(11, Serial.parseInt());
      fan_up_speed = EEPROM.read(11);
      Serial.print(fan_up_speed);
      Serial.println(done);
      delay(pause);

    } else if (a == 7) {
      Serial.print("Circulation (0 - OFF, 1-ON): ");
      while (!Serial.available()) {}
      EEPROM.update(12, Serial.parseInt());
      fanOnOff = EEPROM.read(12);
      if (fanOnOff) {
        Serial.print(" ON");
      } else {
        Serial.print(" OFF");
      }
      Serial.println(done);
      delay(pause);
    }

    else {
      Serial.println(error);
      delay(pause);
    }
  }
   }
}
