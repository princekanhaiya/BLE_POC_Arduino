#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {
  Serial.begin(9600);
  //Use predefined PINS consts
  Wire.begin(D2,D1);
  lcd.init();
  lcd.home();
  lcd.print(" BLE_MOBILE_ACE");
  lcd.backlight();
  pinMode(D3,OUTPUT);
  digitalWrite(D3,LOW);
  delay(3000);
  lcd.clear();
}
char buffer[32];
int charsRead;
String btData="0";

void loop(){
  bt_comm();
}

void bt_comm(){
   if(Serial.available() > 0){
    charsRead = Serial.readBytesUntil('\n', buffer, sizeof(buffer)-1);
    buffer[charsRead] = '\0';
    //Serial.println(buffer);
    lcd.clear();
    lcd.print(buffer);
    btData=buffer;
    
    if(btData=="gateunlocked"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" GATE UNLOCKED");
    }
    if(btData=="gatelocked"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" GATE LOCKED");
    }
    if(btData=="acon"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" AC ON");
    }
    if(btData=="acoff"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" AC OFF");
    }
    if(btData=="sunroofon"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" SUNROOF OPENED");
    }
    if(btData=="sunroofoff"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" SUNROOF CLOSED");
    }
    if(btData=="tailgateon"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("TAIL GATE OPENED");
    }
    if(btData=="tailgateoff"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("TAIL GATE CLOSED");
    }
    if(btData=="ssbon"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Engine On");
    }
    if(btData=="ssboff"){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Engine Off");
    }
  }
}

  


