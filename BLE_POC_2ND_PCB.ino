#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <mcp_can.h>
#include <SPI.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
SoftwareSerial bleSerial(A1, 4); // RX, TX

struct canMessage {
  uint32_t id ;
  byte data[8];
  uint8_t totalFrame;
  uint16_t timeInterval;
} SEARCHCAR, WINDOWUP, WINDOWDOWN, WIPEROPEN, TRUNKOPEN, SUNROOFOPEN, SUNROOFCLOSE, ACON, ACOFF, GATELOCK, GATEUNLOCK;//, IGNON, IGNOFF, CRANKON, CRANKOFF;

MCP_CAN CAN0(10);  // Set CS to pin 10

//receiving can varibles
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
unsigned long previousMillis[5] = {0};
unsigned long currentMillis[5] = {0};

boolean latch_status = false;
boolean break_status = false;
boolean nfc_auth = false;
byte pke_clamp_state = 0x00;
boolean NFC_driver_door_status = false;
boolean NFC_co_driver_door_status = false;
boolean NFC_engine_status = false;

#define buzzerPin 9
#define canIntrruptPin 3
#define greenLED 5
#define redLED 6
#define ssbsw A1
#define ignRel A2
#define crankRel A3
#define autoAuthFail 5000
#define nfcCardPlacmentTime 3500
#define buzzerFreq 1000 //1kHz freq because crystal used in h/w is of 16MHz

char buffer[32];
uint8_t charsRead;
String btData = "0";

void setup() {
  //All CAN messages
  SEARCHCAR       = {0xFE,  { 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00}, 3, 10   };
  WINDOWUP        = {0x6E9, { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 10, 50   };
  WINDOWDOWN      = {0x6EB, { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 10, 10   };
  WIPEROPEN       = {0x182, { 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 10, 40   };
  TRUNKOPEN       = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  SUNROOFOPEN     = {0x633, { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}, 3, 10  };
  SUNROOFCLOSE    = {0x633, { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00}, 3, 10  };
  ACON            = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  ACOFF           = {0x00,  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 0   };
  GATELOCK        = {0xFE, { 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00}, 3, 10};  //XUV700
  GATEUNLOCK      = {0xFE, { 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, 3, 10};  //XUV700
  //  IGNON           = {0x6EB, { 0x2 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 100 };
  //  IGNOFF          = {0x6EB, { 0x3 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 100 };
  //  CRANKON          = {0x6EB, { 0x3 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 100 };
  //  CRANKOFF         = {0x6EB, { 0x3 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 100 };

  pinMode(buzzerPin, OUTPUT);
  pinMode(ssbsw,INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  //for can module
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  pinMode(ignRel, OUTPUT);
  pinMode(crankRel, OUTPUT);
  digitalWrite(ignRel, LOW);
  digitalWrite(crankRel, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);

  Serial.begin(9600);
  bleSerial.begin(9600);

  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");

  //Setting CAN filter and Mask for recieving messages
  pinMode(canIntrruptPin, INPUT_PULLUP);

  CAN0.init_Mask(0, 0, 0x06EF0000);              // Init first mask...all filter bit enable
  CAN0.init_Filt(0, 0, 0x06EA0000);              // Init first filter...//
  CAN0.init_Filt(1, 0, 0x06EB0000);              // Init Second filter...
  CAN0.init_Filt(2, 0, 0x06E90000);              // Init Second filter...

  CAN0.init_Mask(1, 0, 0x0FFF0000);              // Init second mask...
  CAN0.init_Filt(3, 0, 0x02140000);
  CAN0.init_Filt(4, 0, 0x01270000);
  CAN0.init_Filt(5, 0, 0x03530000);

  CAN0.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted

  lcd.init();
  lcd.home();
  lcd.backlight();
  checkPCB();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BLE MOBILE ACCES");
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {

  //Disabling authantication after autoAuthFail seconds.
  //  currentMillis[0] = millis();
  //  if (currentMillis[0] - previousMillis[0] >= autoAuthFail) {
  //    nfc_auth = false;
  //    previousMillis[0] = currentMillis[0];
  //  }
  // char msgString[128];
  //sprintf(msgString, "pke_clamp_sts: 0x%.2x nfc_auth_sts:%d latch_status:%d break_sts:%d NFC_engine_sts:%d NFC_driver_door_sts:%d NFC_co_driver_door_sts:%d", pke_clamp_state, nfc_auth, latch_status, break_status, NFC_engine_status, NFC_driver_door_status, NFC_co_driver_door_status);
  //Serial.println(msgString);
  bt_comm();
  if (!digitalRead(3)) {
    canRead();
  }
  float ssbSwVoltage=(float) analogRead(ssbsw)*5.0/1023.0;
  Serial.println(ssbSwVoltage);

  if(ssbSwVoltage>1.8 && ssbSwVoltage < 2.8){
    digitalWrite(redLED,LOW);
    digitalWrite(greenLED,HIGH);
  }
  else{
   digitalWrite(redLED,HIGH);
   digitalWrite(greenLED,LOW);
  }
}

void bt_comm() {
  if (bleSerial.available() > 0) {
    charsRead = bleSerial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    buffer[charsRead] = '\0';
    delay(100);
    Serial.println(buffer);
    btData = buffer;

    if (btData == "search") {
      canWrite(SEARCHCAR, "search");
      lcdWrite(" SEARCHING CAR  ");
    }
    if (btData == "windowup") {
      canWrite(WINDOWUP, "windowup");
      lcdWrite(" POWER WINDOW UP");
    }
    if (btData == "windowdown") {
      canWrite(WINDOWDOWN, "windowdown");
      lcdWrite("POWER WINDOW DWN");
    }
    if (btData == "wiperopen") {
      canWrite(WIPEROPEN, "wiperopen");
      lcdWrite(" WIPER RUNNING  ");
    }
    if (btData == "trunkopen") {
      canWrite(TRUNKOPEN, "trunkopen");
      lcdWrite(" TRUNK OPENING  ");
    }
    if (btData == "sunroofopen") {
      canWrite(SUNROOFOPEN, "sunroofopen");
      lcdWrite(" SUNROOF OPENED ");
    }
    if (btData == "sunroofclose") {
      canWrite(SUNROOFCLOSE, "sunroofclose");
      lcdWrite(" SUNROOF CLOSED ");
    }
    if (btData == "gateopen") {
      canWrite(GATEUNLOCK, "gateopen");
      lcdWrite(" GATE UNLOCKED  ");
    }
    if (btData == "gateclose") {
      canWrite(GATELOCK, "gateclose");
      lcdWrite("  GATE LOCKED   ");
    }
    if (btData == "acopen") {
      canWrite(ACON, "acopen");
      lcdWrite("     AC ON      ");
    }
    if (btData == "acclose") {
      canWrite(ACOFF, "acclose");
      lcdWrite("     AC OFF     ");
    }
    if (btData == "ignon") {
      //      canWrite(IGNON, "ignon");
      lcdWrite("  IGNITION ON   ");
      digitalWrite(ignRel, HIGH);
    }
    if (btData == "ignoff") {
      //      canWrite(IGNOFF, "ignoff");
      digitalWrite(ignRel, LOW);
      lcdWrite("  IGNITION OFF  ");
    }
    if (btData == "crankon") {
      //      canWrite(CRANKON, "crankon");
      digitalWrite(crankRel, HIGH);
      lcdWrite("  CRANK ON  ");
    }
    if (btData == "crankoff") {
      //      canWrite(CRANKOFF, "crankoff");
      digitalWrite(crankRel, LOW);
      lcdWrite("  CRANK OFF  ");
    }
    if (btData == "reset") {
      Serial.println("Wait ECU is resetting");
      delay(100);
      resetFunc();  //call reset
    }
  }
}
void checkPCB() {
  lcd.setCursor(0, 0);
  lcd.print("CHECKING PCB...");
  delay(10);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  tone(buzzerPin, buzzerFreq); // Send 1KHz sound signal...
  lcd.setCursor(0, 1);
  lcd.print("BUZZER OK!");
  delay(100);           // ...for 0.5 sec
  noTone(buzzerPin);     // Stop sound...
  delay(100);
}
void canWrite(canMessage message, String strAck) {
  // send data:  to given ID, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  uint8_t countAck = 0;
  for (uint8_t i = 0; i < message.totalFrame; i++) {
    byte ack = CAN0.sendMsgBuf(message.id, 0, 8, message.data);
    if (ack == CAN_OK) {
      countAck++;
      Serial.println("Message Sent Successfully!");
    }
    else {
      Serial.print("Error Sending Message...ACK-");
      Serial.println(ack);
    }
    delay(message.timeInterval);
  }
  delay(100);
  if (countAck == message.totalFrame) {
    bleSerial.print(strAck);
    Serial.println(strAck);
  }
  else {
    bleSerial.print("ackerror");
    Serial.println("ackerror");

    CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ);
    CAN0.init_Mask(0, 0, 0x03FF0000);              // Init first mask...all filter bit enable
    CAN0.init_Filt(0, 0, 0x06EA0000);              // Init first filter...
    CAN0.init_Mask(1, 0, 0x03FF0000);              // Init second mask...
    CAN0.init_Filt(1, 0, 0x06EB0000);              // Init Second filter...
    CAN0.init_Filt(2, 0, 0x06E90000);              // Init Second filter...

    CAN0.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted
    delay(400);
  }

}
void lcdWrite(String data) {
  lcd.clear();
  lcd.setCursor(0, 0);
  tone(buzzerPin, buzzerFreq);
  delay(200);
  noTone(buzzerPin);
  lcd.print(data);
}
void canRead() {
  CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
  Serial.print("CAN ID: 0x"); Serial.print(rxId, HEX);
  for (byte i = 0; i < len; i++) {
    Serial.print(" ");
    Serial.print(rxBuf[i], HEX);
  }
  Serial.println();

  switch (rxId) {
    case 0x353://PKE Clamp state op
      pke_clamp_state = rxBuf[0];
      break;
    case 0x214://MBFM Latch status
      if (rxBuf[5] == 0x40)
        latch_status = true;
      else
        latch_status = false;
      break;
    case 0x127://EMS break status
      if (rxBuf[3] == 0x08)
        break_status = true;
      else
        break_status = false;
      break;
    case 0x6EB://CAN ID = 0x6EB for Engine Ignition.
      if (rxBuf[0] == 0x03) {
        currentMillis[3] = millis();
        if (currentMillis[3] - previousMillis[3] <= nfcCardPlacmentTime) {
          Serial.println("returning from ign handlling");
          return;
        }
        previousMillis[3] = currentMillis[3];
        nfc_auth = !nfc_auth;
        if (nfc_auth) {
          lcdWrite("  IGNITION ON   ");
          digitalWrite(ignRel, HIGH);
          if (break_status) {
            digitalWrite(crankRel, HIGH);
            lcdWrite("  CRANK ON  ");
            break_status=false;
          }
        }
        if (!nfc_auth) {
          lcdWrite("  IGNITION OFF ");
          digitalWrite(crankRel, LOW);
          digitalWrite(ignRel, LOW);
          break_status=false;
        }
      }
      break;
    case 0x6EA://CAN ID = 0x6EA for co driver side door access.
      if (rxBuf[0] == 0x03) {
        currentMillis[2] = millis();
        if (currentMillis[2] - previousMillis[2] <= nfcCardPlacmentTime) {
          Serial.println("returning co door handling");
          return;
        }
        previousMillis[2] = currentMillis[2];
        NFC_driver_door_status = !NFC_driver_door_status;
        if (NFC_driver_door_status) {
          Serial.println("Driver door open");
          canWrite(GATEUNLOCK, "gateopen");
          lcdWrite(" GATE UNLOCKED  ");
        }
        if (!NFC_driver_door_status) {
          Serial.println("Driver door close");
          canWrite(GATEUNLOCK, "gateclose");
          lcdWrite(" GATE LOCKED  ");
        }
      }
      break;
    case 0x6E9://CAN ID = 0X6E9 for driver side door access.
      if (rxBuf[0] == 0x03) {
        currentMillis[1] = millis();
        if (currentMillis[1] - previousMillis[1] <= nfcCardPlacmentTime) {
          Serial.println("returning from door handlling");
          return;
        }
        previousMillis[1] = currentMillis[1];
        NFC_driver_door_status = !NFC_driver_door_status;
        if (NFC_driver_door_status) {
          Serial.println("Driver door open");
          canWrite(GATEUNLOCK, "gateopen");
          lcdWrite(" GATE UNLOCKED  ");
        }
        if (!NFC_driver_door_status) {
          Serial.println("Driver door close");
          canWrite(GATEUNLOCK, "gateclose");
          lcdWrite(" GATE LOCKED  ");
        }
      }
      break;
  }
  //  char msgString[128];
  //  sprintf(msgString, "pke_clamp_sts: 0x%.2x nfc_auth_sts:%d latch_status:%d break_sts:%d NFC_engine_sts:%d NFC_driver_door_sts:%d NFC_co_driver_door_sts:%d", pke_clamp_state, nfc_auth, latch_status, break_status, NFC_engine_status, NFC_driver_door_status, NFC_co_driver_door_status);
  //  Serial.println(msgString);
  rxId = 0x00;
  for (int i = 0; i < 8; i++)
    rxBuf[i] = 0x00;
}
void messgeHandle() {
  //  boolean latch_status = false;
  //  boolean break_status = false;
  //  boolean nfc_auth = false;
  //  byte pke_clamp_state = 0x00;
  //  boolean NFC_driver_door_status = false;
  //  boolean NFC_co_driver_door_status = false;
  //  boolean NFC_engine_status = false;
  currentMillis[3] = millis();
  if (currentMillis[3] - previousMillis[3] <= autoAuthFail) {
    Serial.println("returning from ign handlling");
    return;
  }
  previousMillis[3] = currentMillis[3];


}
