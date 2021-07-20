#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
SoftwareSerial bleSerial(A3, A2); // RX, TX

struct canMessage {
  uint16_t id ;
  byte data[8];
  uint8_t totalFrame;
  uint16_t timeInterval;
} SEARCHCAR, WINDOWUP, WINDOWDOWN, WIPEROPEN, TRUNKOPEN, SUNROOFOPEN, SUNROOFCLOSE, ACON, ACOFF, GATELOCK, GATEUNLOCK, SSBON, SSBOFF;


//can integration
#include <mcp_can.h>
#include <SPI.h>
MCP_CAN CAN0(10);  // Set CS to pin 10

//receiving can varibles
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgBuffer[128];                        // Array to store serial string

const uint8_t ledIndicator[5] = {7, 6, 5, 4, 3};
#define buzzerPin 8
#define bleVccPin A0
#define bleGndPin A1
#define canIntrruptPin 2
#define buzzerFreq 1000 //1kHz freq because crystal used in h/w is of 16MHz

char buffer[32];
uint8_t charsRead;
String btData = "0";

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
  //  GATELOCK        = {0x6E9, { 0x2 , 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00}, 1, 100 }; XUV500
  //  GATEUNLOCK      = {0x6E9, { 0x3 , 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, 1, 100 }; //XUV500
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
  
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  pinMode(canIntrruptPin, INPUT);                       // Setting pin 2 for /INT input

  digitalWrite(bleGndPin, LOW);
  analogWrite(bleVccPin, 168);

  Serial.begin(115200);
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

  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
}


void loop() {
  bt_comm();
  //recCanMessage();
}

void bt_comm() {
  if (bleSerial.available() > 0) {
    charsRead = bleSerial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    //Serial.available() > 0
    //charsRead = Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    buffer[charsRead] = '\0';
    Serial.println(buffer);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(buffer);
    btData = buffer;

    if (btData == "search")       actionOnBtData("search",       SEARCHCAR,    " SEARCHING CAR  ",  "RIGHT",         0,  false);
    if (btData == "windowup")     actionOnBtData("windowup",     WINDOWUP,     " POWER WINDOW UP",  "RIGHT",         0,  false);
    if (btData == "windowdown")   actionOnBtData("windowdown",   WINDOWDOWN,   "POWER WINDOW DWN",  "LEFT",          0,  false);
    if (btData == "wiperopen")    actionOnBtData("wiperopen",    WIPEROPEN,    " WIPER RUNNING  ",  "CONSTANTBLINK", 0,  false);
    if (btData == "trunkopen")    actionOnBtData("trunkopen",    TRUNKOPEN,    " TRUNK OPENING  ",  "CONSTANT",      3,  true );
    if (btData == "sunroofopen")  actionOnBtData("sunroofopen",  SUNROOFOPEN,  " SUNROOF OPENED ",  "CONSTANT",      2,  true );
    if (btData == "sunroofclose") actionOnBtData("sunroofclose", SUNROOFCLOSE, " SUNROOF CLOSED ",  "CONSTANT",      2,  false);
    if (btData == "gateopen")     actionOnBtData("gateopen",     GATEUNLOCK,   " GATE UNLOCKED  ",  "CONSTANT",      0,  true );
    if (btData == "gateclose")    actionOnBtData("gateclose",    GATELOCK,     "  GATE LOCKED   ",  "CONSTANT",      0,  false);
    if (btData == "acopen")       actionOnBtData("acopen",       ACON,         "     AC ON      ",  "CONSTANT",      1,  true );
    if (btData == "acclose")      actionOnBtData("acclose",      ACOFF,        "     AC OFF     ",  "CONSTANT",      1,  false);
    if (btData == "ssbon")        actionOnBtData("ssbon",        SSBON,        "   ENGINE ON    ",  "CONSTANT",      4,  true );
    if (btData == "ssboff")       actionOnBtData("ssboff",       SSBOFF,       "   ENGINE OFF   ",  "CONSTANT",      4,  false);
  }
}

void checkPCB() {
  lcd.setCursor(0, 0);
  lcd.print("CHECKING PCB...");
  delay(10);
  for (uint8_t i = 0; i < 5; i++) {
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
  delay(300);
}

void sendDataToCAN(canMessage message, String ack) {
  // send data:  to given ID, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  uint8_t countAck = 0;
  for (uint8_t i = 0; i < message.totalFrame; i++) {
    byte ack = CAN0.sendMsgBuf(message.id, 0, 8, message.data);
    if (ack == CAN_OK) {
      countAck++;
      sprintf(msgBuffer, "Message Sent Successfully! CAN id-0x%.3lX, Data-0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X, Standard CAN Frame, DLC- 8Byte", message.id, message.data[0], message.data[1], message.data[2], message.data[3], message.data[4], message.data[5], message.data[6], message.data[7]);
      Serial.println(msgBuffer);
    }
    else {
      sprintf(msgBuffer, "Error Sending Message...ACK-%d", ack);
      Serial.println(msgBuffer);
    }
    delay(message.timeInterval);
  }
  delay(100);
  if (countAck == message.totalFrame){
    bleSerial.print(ack);
    Serial.println(ack);
  }
  else{
    bleSerial.print("ackerror");
    Serial.println("ackerror");
  }

}
void ledScrolling(String direction, uint8_t whichLed, boolean ledStatus) {
  if (direction == "CONSTANT")
    digitalWrite(ledIndicator[whichLed], ledStatus);

  if (direction == "RIGHT") {
    for (uint8_t i = 0; i < 5; i++) {
      digitalWrite(ledIndicator[i], HIGH);
      delay(50);
      digitalWrite(ledIndicator[i], LOW);
      delay(50);
    }
  }
  if (direction == "LEFT") {
    for (uint8_t i = 4; i >= 0; i--) {
      digitalWrite(ledIndicator[i], HIGH);
      delay(50);
      digitalWrite(ledIndicator[i], LOW);
      delay(50);
    }
  }
  if (direction == "CONSTANTBLINK") {
    for (uint8_t i = 0; i < 5; i++) {
      digitalWrite(ledIndicator[i], HIGH);
    }
    delay(100);
    for (uint8_t i = 0; i < 5; i++) {
      digitalWrite(ledIndicator[i], LOW);
    }
  }
}

//param@ actionOnBtData(String canD, String lcdPrintData, String ledPattern)
void actionOnBtData(String ack, canMessage message, String lcdPrintData, String ledPattern , uint8_t whichLed, boolean ledStatus) {
  lcd.clear();
  lcd.setCursor(0, 0);
  tone(buzzerPin, buzzerFreq);
  delay(200);
  noTone(buzzerPin);
  lcd.print(lcdPrintData);
  sendDataToCAN(message, ack);
  ledScrolling(ledPattern, whichLed, ledStatus);
}

void recCanMessage() {
  if (CAN_MSGAVAIL == CAN0.checkReceive()){
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    if ((rxId & 0x80000000) == 0x80000000)    // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgBuffer, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgBuffer, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
    Serial.print(msgBuffer);
    if ((rxId & 0x40000000) == 0x40000000) {  // Determine if message is a remote request frame.
      sprintf(msgBuffer, " REMOTE REQUEST FRAME");
      Serial.print(msgBuffer);
    } else {
      for (byte i = 0; i < len; i++) {
        sprintf(msgBuffer, " 0x%.2X", rxBuf[i]);
        Serial.print(msgBuffer);
      }
    }
    Serial.println();
  }
}
