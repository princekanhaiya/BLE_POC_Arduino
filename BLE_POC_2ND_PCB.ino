#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
SoftwareSerial bleSerial(A3, A2); // RX, TX

struct canMessage {
  int id;
  byte data[8];
  unsigned int totalFrame;
  unsigned int timeInterval;
} SEARCHCAR, WINDOWUP, WINDOWDOWN, WIPEROPEN, TRUNKOPEN, SUNROOFOPEN, SUNROOFCLOSE, ACON, ACOFF, GATELOCK, GATEUNLOCK, SSBON, SSBOFF;


//can integration
#include <mcp_can.h>
#include <SPI.h>
MCP_CAN CAN0(10);  // Set CS to pin 10

const int ledIndicator[5] = {6, 5, 4, 3, 2};
const int buzzerPin = 7;
const int bleVccPin = A0;
const int bleGndPin = A1;

char buffer[32];
int charsRead;
String btData = "0";
const int buzzerFreq = 1000; //1kHz fre because crystal used in h/w is of 8MHz
void setup() {
  //All CAN messages
  SEARCHCAR       = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  WINDOWUP        = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  WINDOWDOWN      = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  WIPEROPEN       = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  TRUNKOPEN       = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  SUNROOFOPEN     = {0x633, { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}, 3, 10  };
  SUNROOFCLOSE    = {0x633, { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00}, 3, 10  };
  ACON            = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  ACOFF           = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
//  GATELOCK        = {0x6E9, { 0x2 , 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00}, 1, 100 };
//  GATEUNLOCK      = {0x6E9, { 0x3 , 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, 1, 100 };
  GATELOCK        = {0xFE, { 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00}, 3, 10};
  GATEUNLOCK      = {0xFE, { 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, 3, 10};
  SSBON           = {0x6EB, { 0x2 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 100 };
  SSBOFF          = {0x6EB, { 0x3 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 100 };

  pinMode(ledIndicator[0], OUTPUT);
  pinMode(ledIndicator[1], OUTPUT);
  pinMode(ledIndicator[2], OUTPUT);
  pinMode(ledIndicator[3], OUTPUT);
  pinMode(ledIndicator[4], OUTPUT);
  //BLE Module power supply
  pinMode(bleGndPin, OUTPUT);
  pinMode(bleVccPin, OUTPUT);
 
  digitalWrite(bleGndPin, LOW);
  analogWrite(bleVccPin, 168);

  Serial.begin(9600);
  bleSerial.begin(9600);

  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  CAN0.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted

  lcd.init();
  lcd.home();
  lcd.backlight();
  checkPCB();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BLE MOBILE ACCES");
}


void loop() {
  bt_comm();
}

void bt_comm() {
  if (bleSerial.available() > 0) {
    charsRead = bleSerial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    buffer[charsRead] = '\0';
    Serial.println(buffer);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(buffer);
    btData = buffer;

    if (btData == "search")       actionOnBtData(SEARCHCAR,    " SEARCHING CAR  ",  "RIGHT",         0,  false);
    if (btData == "windowup")     actionOnBtData(WINDOWUP,     " POWER WINDOW UP",  "RIGHT",         0,  false);
    if (btData == "windowdown")   actionOnBtData(WINDOWDOWN,   "POWER WINDOW DWN",  "LEFT",          0,  false);
    if (btData == "wiperopen")    actionOnBtData(WIPEROPEN,    " WIPER RUNNING  ",  "no", 0,  false);
    if (btData == "trunkopen")    actionOnBtData(TRUNKOPEN,    " TRUNK OPENING  ",  "CONSTANT",      3,  true );
    if (btData == "sunroofopen")  actionOnBtData(SUNROOFOPEN,  " SUNROOF OPENED ",  "CONSTANT",      2,  true );
    if (btData == "sunroofclose") actionOnBtData(SUNROOFCLOSE, " SUNROOF CLOSED ",  "CONSTANT",      2,  false);
    if (btData == "gateopen")     actionOnBtData(GATEUNLOCK,   " GATE UNLOCKED  ",  "CONSTANT",      0,  true );
    if (btData == "gateclose")    actionOnBtData(GATELOCK,     "  GATE LOCKED   ",  "CONSTANT",      0,  false);
    if (btData == "acopen")       actionOnBtData(ACON,         "     AC ON      ",  "CONSTANT",      1,  true );
    if (btData == "acclose")      actionOnBtData(ACOFF,        "     AC OFF     ",  "CONSTANT",      1,  false);
    if (btData == "ssbon")        actionOnBtData(SSBON,        "   ENGINE ON    ",  "CONSTANT",      4,  true );
    if (btData == "ssboff")       actionOnBtData(SSBOFF,       "   ENGINE OFF   ",  "CONSTANT",      4,  false);
  }
}

void checkPCB() {
  lcd.setCursor(0, 0);
  lcd.print("CHECKING PCB...");
  delay(500);
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledIndicator[i], HIGH);
    lcd.setCursor(0, 1);
    char temp = i + 49;
    lcd.print("INDICATOR ");
    lcd.setCursor(10, 1);
    lcd.print(temp);
    lcd.setCursor(12, 1);
    lcd.print("OK!");
    delay(100);
    digitalWrite(ledIndicator[i], LOW);
    delay(100);
  }

  lcd.setCursor(0, 1);
  lcd.print("                ");
  tone(buzzerPin, buzzerFreq); // Send 1KHz sound signal...
  lcd.setCursor(0, 1);
  lcd.print("BUZZER OK!");
  delay(300);           // ...for 0.5 sec
  noTone(buzzerPin);     // Stop sound...
  delay(500);
}

boolean sendDataToCAN(canMessage message) {
  // send data:  to given ID, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  for (int i = 0; i < message.totalFrame; i++) {
    byte ack = CAN0.sendMsgBuf(message.id, 0, 8, message.data);
    if (ack == CAN_OK) {
      Serial.print("Message Sent Successfully! Address-0x");
      Serial.print(message.id, HEX);
      Serial.print(", Standard CAN Frame, Data Length- 8Byte, Data Sent- ");
      for (int j = 0; j < 8; j++) {
        Serial.print(message.data[j]);
        Serial.print(" ");
      }
      Serial.print("ACK ");
      Serial.print(ack);
      Serial.println();
    }
    else {
      Serial.print("Error Sending Message...ACK-");  
      Serial.print(ack);
    }
    delay(message.timeInterval);
  }
  
}

void ledScrolling(String direction, int whichLed, boolean ledStatus) {
  if(direction=="CONSTANT")
    digitalWrite(ledIndicator[whichLed],ledStatus);

  if (direction == "RIGHT") {
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledIndicator[i], HIGH);
      delay(50);
      digitalWrite(ledIndicator[i], LOW);
      delay(50);
    }
  }
  if (direction == "LEFT") {
    for (int i = 4; i >= 0; i--) {
      digitalWrite(ledIndicator[i], HIGH);
      delay(50);
      digitalWrite(ledIndicator[i], LOW);
      delay(50);
    }
  }
  if (direction == "CONSTANTBLINK") {
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledIndicator[i], HIGH);
    }
    delay(100);
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledIndicator[i], LOW);
    }
  }
}

//param@ actionOnBtData(String canD, String lcdPrintData, String ledPattern)
void actionOnBtData(canMessage message, String lcdPrintData, String ledPattern , int whichLed, boolean ledStatus) {
  lcd.clear();
  lcd.setCursor(0, 0);
  tone(buzzerPin, buzzerFreq);
  delay(200);
  noTone(buzzerPin);
  lcd.print(lcdPrintData);
  sendDataToCAN(message);
  ledScrolling(ledPattern, whichLed, ledStatus);
}
