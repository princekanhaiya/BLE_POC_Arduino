#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <mcp_can.h>
#include <SPI.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
SoftwareSerial bleSerial(4, A1); // RX, TX

struct canMessage {
  uint16_t id ;
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
boolean ssbButtonStatus = false;
boolean ignitionStatus = false;
boolean crankStatus = false;
boolean gearStatus = false;
boolean ledState = false;
boolean authFailState = false;
byte blinkCount = 0;
byte buttonPushCounter = 0;

#define buzzerPin 9
#define canIntrruptPin 3
#define greenLED 5
#define redLED 6
#define ssbsw A1
#define ignRel A2
#define crankRel A3
#define autoAuthFail 40000
#define nfcCardPlacmentTime 2000
#define buzzerFreq 1000 //1kHz freq because crystal used in h/w is of 16MHz

//All CAN Messages IDs
#define GEAR_STATUS_ID                0x278
#define PKE_CLAMP_STATE_OP_ID         0x353
#define LATCH_STATUS_ID               0x214
#define BREAK_STATUS_ID               0x127
#define NFC_ENGINE_MODULE_ID          0x6EB
#define NFC_DRIVER_DOOR_MODULE_ID     0X6E9
#define NFC_CO_DRIVER_DOOR_MODULE_ID  0x6EA
#define VEHICLE_SPEED_ID

#define SEARCHCAR_ID                  0x0FE
#define WINDOWUP_ID                   0x6E9
#define WINDOWDOWN_ID                 0x6EB
#define WIPEROPEN_ID                  0x182
#define TRUNKOPEN_ID                  0x000
#define SUNROOFOPEN_ID                0x633
#define SUNROOFCLOSE_ID               0x633
#define ACON_ID                       0x000
#define ACOFF_ID                      0x000
#define GATELOCK_ID                   0x0FE
#define GATEUNLOCK_ID                 0x0FE

char buffer[32];
uint8_t charsRead;
String btData = "0";
float ssbPreviousVoltage = 5.0;
float ssbCurrentVoltage  = 5.0;

void setup() {
  //All CAN messages
  SEARCHCAR       = {SEARCHCAR_ID,    { 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00}, 03, 10 };
  WINDOWUP        = {WINDOWUP_ID,     { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 10, 50 };
  WINDOWDOWN      = {WINDOWDOWN_ID,   { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 10, 10 };
  WIPEROPEN       = {WIPEROPEN_ID,    { 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 10, 40 };
  TRUNKOPEN       = {TRUNKOPEN_ID,    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 01, 00 };
  SUNROOFOPEN     = {SUNROOFOPEN_ID,  { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}, 03, 10 };
  SUNROOFCLOSE    = {SUNROOFCLOSE_ID, { 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00}, 03, 10 };
  ACON            = {ACON_ID,         { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 01, 00 };
  ACOFF           = {ACOFF_ID,        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 01, 00 };
  GATELOCK        = {GATELOCK_ID,     { 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00}, 03, 10 };
  GATEUNLOCK      = {GATEUNLOCK_ID,   { 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, 03, 10 };

  pinMode(buzzerPin, OUTPUT);
  pinMode(ssbsw, INPUT);
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
  CAN0.init_Filt(3, 0, 0x02780000);

  CAN0.init_Mask(1, 0, 0x0FFF0000);              // Init second mask...
  CAN0.init_Filt(4, 0, 0x02140000);
  CAN0.init_Filt(5, 0, 0x01270000);
  CAN0.init_Filt(6, 0, 0x03530000);


  CAN0.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted

  lcd.init();
  lcd.home();
  lcd.backlight();
  //checkPCB();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BLE MOBILE ACCES");
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {
  //  Disabling authantication after autoAuthFail seconds.
  if (nfc_auth) {
    currentMillis[0] = millis();
    if (currentMillis[0] - previousMillis[0] >= autoAuthFail) {
      nfc_auth = false;
      lcdWrite(" AUTH FAILED");
      lcd.setCursor(0, 1);
      lcd.print(" PLACE AGAIN TAG");
      previousMillis[0] = currentMillis[0];
    }
  }
  // char msgString[128];
  //sprintf(msgString, "pke_clamp_sts: 0x%.2x nfc_auth_sts:%d latch_status:%d break_sts:%d NFC_engine_sts:%d NFC_driver_door_sts:%d NFC_co_driver_door_sts:%d", pke_clamp_state, nfc_auth, latch_status, break_status, NFC_engine_status, NFC_driver_door_status, NFC_co_driver_door_status);
  //Serial.println(msgString);
  bt_comm();
  if (!digitalRead(3)) {
    canRead();
  }
  if (ssbPreviousVoltage != ssbCurrentVoltage) {
    ssbPreviousVoltage = ssbCurrentVoltage = (float) analogRead(ssbsw) * 5.0 / 1023.0;
  }
  ssbCurrentVoltage = (float) analogRead(ssbsw) * 5.0 / 1023.0;
  if ((ssbPreviousVoltage - ssbCurrentVoltage) > 2) {
    Serial.print(ssbCurrentVoltage);
    Serial.print(" buttonPushCounter");
    buttonPushCounter++;
    Serial.println(buttonPushCounter);
    if (nfc_auth && !ignitionStatus && !crankStatus) {
      digitalWrite(redLED, HIGH);
      lcdWrite("  IGNITION ON   ");
      digitalWrite(ignRel, HIGH);
      ignitionStatus = true;
      if (break_status && !crankStatus && gearStatus) {
        crank();
      }
      return;
    }
    if (ignitionStatus && crankStatus) {
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, LOW);
      digitalWrite(crankRel, LOW);
      digitalWrite(ignRel, LOW);
      lcdWrite("   ENGINE OFF   ");
      ignitionStatus=false;
      crankStatus = false;
      nfc_auth=false;
      return;
    }
    if (ignitionStatus && break_status && gearStatus) {
      crank();
      return;
    }
    if (ignitionStatus) {
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, LOW);
      lcdWrite("  IGNITION OFF   ");
      digitalWrite(ignRel, LOW);
      ignitionStatus = false;
      return;
    }
    if (!nfc_auth && !ignitionStatus) {
      lcdWrite(" PLACE NFC CARD");
      lcd.setCursor(0, 1);
      lcd.print(" AGAIN FOR AUTH");
      authFailState = true;
      blinkCount = 0;
    }
    ssbPreviousVoltage = ssbCurrentVoltage;
  }

  if (ignitionStatus && !crankStatus) {
    digitalWrite(greenLED, break_status);
  }

  if (authFailState && !ignitionStatus ) {
    currentMillis[4] = millis();
    if (currentMillis[4] - previousMillis[4] >= 200) {
      previousMillis[4] = currentMillis[4];
      if (ledState == LOW) {
        ledState = HIGH;
        blinkCount++;
      } else {
        ledState = LOW;
      }
      digitalWrite(redLED, ledState);
    }
    if (blinkCount == 10) {
      authFailState = false;
      digitalWrite(redLED, LOW);
    }
  }
  //do something on multiple press of buttons
  if (buttonPushCounter > 2 && nfc_auth && gearStatus && !ignitionStatus && !crankStatus) {
    digitalWrite(redLED, HIGH);
    lcdWrite("  IGNITION ON   ");
    digitalWrite(ignRel, HIGH);
    ignitionStatus = true;
    crank();
    buttonPushCounter = 0;
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
  tone(buzzerPin, buzzerFreq); // Send 1KHz sound signal...
  delay(200);           // ...for 0.5 sec
  noTone(buzzerPin);     // Stop sound...
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
    delay(100);
    resetFunc();
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
    case PKE_CLAMP_STATE_OP_ID://PKE Clamp state op
      pke_clamp_state = rxBuf[0];
      break;
    case LATCH_STATUS_ID://MBFM Latch status
      if (rxBuf[5] == 0x40)
        latch_status = true;
      else
        latch_status = false;
      break;
    case BREAK_STATUS_ID://EMS break status
      if (rxBuf[3] == 0x08)
        break_status = true;
      else
        break_status = false;
      break;
    case GEAR_STATUS_ID://Gear Status
      if (rxBuf[2] == 0x09)
        gearStatus = true;
      else
        gearStatus = false;
      break;
    case NFC_ENGINE_MODULE_ID://CAN ID = 0x6EB for Engine Ignition.
      if (rxBuf[0] == 0x03) {
        currentMillis[3] = millis();
        if (currentMillis[3] - previousMillis[3] <= 500) {
          Serial.println("returning from authentication");
          return;
        }
        previousMillis[0]=millis();
        previousMillis[3] = currentMillis[3];
        nfc_auth = true;
        buttonPushCounter = 0;
        lcdWrite(" AUTHENTICATED");
      }
      break;
    case NFC_CO_DRIVER_DOOR_MODULE_ID://CAN ID = 0x6EA for co driver side door access.
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
    case NFC_DRIVER_DOOR_MODULE_ID://CAN ID = 0X6E9 for driver side door access.
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
void crank() {
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  delay(1000);
  digitalWrite(crankRel, HIGH);
  crankStatus = true;
  lcdWrite("  CRANK ON  ");
  delay(3000);
  digitalWrite(crankRel, LOW);
  lcdWrite("  CRANK OFF  ");
}
