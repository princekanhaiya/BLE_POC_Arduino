// Including libraries
#include <same51_can.h>
#include "wiring_private.h"

// Macro functions
#define CHECK_BIT_IN_BYTE(BYTE, BIT_NO) (((BYTE) >> (BIT_NO)) & 1)

// Hardwired pins
#define SSB_GREEN 12
#define SSB_RED A0
#define SSB_GND A1
#define BUZZER A2
#define SSB_SW A3
#define BOARD_GPIO_VOLTAGE 3.3

#define BUZZERFREQ 1000
#define BUZZER_DELAY_TIME 100
#define STD_FRAME 0
#define EXT_FRAME 1
#define CAN_DATA_LEN 8

// All CAN Messages IDs
#define GEAR_STATUS_ID 0x278

// Making all objects
SAME51_CAN CAN;

#define CAN_MSG_ID 0x585

float ssbPreviousVoltage = BOARD_GPIO_VOLTAGE;
float ssbCurrentVoltage = BOARD_GPIO_VOLTAGE;

// receiving CAN varibles
uint32_t rxId;
uint8_t ret;
uint8_t len = 0;
uint8_t rxBuf[8];
unsigned long previousMillis[5] = {0};
unsigned long currentMillis[5] = {0};

boolean ssbButtonStatus = false;
boolean ledState = false;
boolean timeout = true;

byte blinkCount = 0;
byte buttonPushCounter = 0;
uint32_t countSSB = 0;

struct CANMessage{
  uint16_t id;
  byte data[8];
  uint8_t totalFrame;
  uint8_t timeInterval;
} DYNAMIC_MODE,CITY_MODE,SPORTS_MODE;

void CANWrite(CANMessage message, String strAck);

// Preparing the code
void setup(){
  // All CAN SINGNALS
  DYNAMIC_MODE = {CAN_MSG_ID, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 01, 0};
  CITY_MODE = {CAN_MSG_ID,    {0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00}, 01, 0};
  SPORTS_MODE = {CAN_MSG_ID,  {0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00}, 01, 0};

  // Configuring all GPIOs pins
  pinMode(BUZZER, OUTPUT);
  pinMode(SSB_GREEN, OUTPUT);
  pinMode(SSB_RED, OUTPUT);
  pinMode(SSB_GND, OUTPUT);

  pinMode(SSB_SW, INPUT_PULLUP);

  // Setting and starting all the serial baud rate
  Serial.begin(115200);
  delay(100);

  if ((CAN.begin(MCP_ANY, CAN_500KBPS, MCAN_MODE_CAN)) == CAN_OK)
  {
    Serial.println("CAN Initialized Successfully!");
    buzzerSound(BUZZER_DELAY_TIME);
    delay(100);
    buzzerSound(BUZZER_DELAY_TIME);
  }
  else
  {
    Serial.println("Error Initializing CAN...");
    delay(100);
  }

  // Change to normal mode to allow messages to be transmitted
  CAN.setMode(MCP_NORMAL);

  // Initial setup for all the configured pins
  digitalWrite(SSB_GREEN, LOW);
  digitalWrite(SSB_RED, LOW);
  digitalWrite(SSB_GND, LOW);
  digitalWrite(BUZZER, LOW);

}

void loop(){

  //SSB Code start from here
  // Eleminiting low->high transistion in the SSB
  if (ssbPreviousVoltage != ssbCurrentVoltage){
    ssbPreviousVoltage = ssbCurrentVoltage = (float)analogRead(SSB_SW) * BOARD_GPIO_VOLTAGE / 1023.0;
    countSSB = 0;
  }
  ssbCurrentVoltage = (float)analogRead(SSB_SW) * BOARD_GPIO_VOLTAGE / 1023.0;
  /*Performing the actions When SSB is pressed and with voltage differance of 2v
    Sginal edge detection________________         ________ _ _ _ _5V
                                        |         |
    Performing action on this edge  --->|         | }-------->2V differance ie when button is pressed
                                        |_________| _ _ _ _ _ _ _ 0V         */
  if ((ssbPreviousVoltage - ssbCurrentVoltage) > 1){
    Serial.print(F("Previous Voltage->"));
    Serial.print(ssbPreviousVoltage);
    Serial.print(F(" Current Voltage->"));
    Serial.print(ssbCurrentVoltage);
    Serial.print(F(" buttonPushCounter->"));
    buttonPushCounter++;
    Serial.println(buttonPushCounter);
  }

  switch(buttonPushCounter){
    case 1:
      digitalWrite(SSB_RED, HIGH);
      digitalWrite(SSB_GREEN, HIGH);
      currentMillis[0] = millis();
      if (currentMillis[0] - previousMillis[0] >= 500){
        previousMillis[0] = currentMillis[0];
        CANWrite(DYNAMIC_MODE, F("gateclose"));
      }
      break;
      
    case 2:
      digitalWrite(SSB_RED, LOW);
      digitalWrite(SSB_GREEN, HIGH);
      currentMillis[1] = millis();
      if (currentMillis[1] - previousMillis[1] >= 500){
        previousMillis[1] = currentMillis[1];
        CANWrite(CITY_MODE, F("gateclose"));
      }
      break;

    case 3:
      digitalWrite(SSB_RED, HIGH);
      digitalWrite(SSB_GREEN, LOW);
      currentMillis[2] = millis();
      if (currentMillis[2] - previousMillis[2] >= 500){
        previousMillis[2] = currentMillis[2];
        CANWrite(SPORTS_MODE, F("gateclose"));
      }
      break;

    default:
      buttonPushCounter=1;
      break;
  }
}

void CANWrite(CANMessage message, String strAck){
  // send data:  to given ID, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  uint8_t countAck = 0;
  for (uint8_t i = 0; i < message.totalFrame; i++)
  {
    byte ack = CAN.sendMsgBuf(message.id, STD_FRAME, CAN_DATA_LEN, message.data);
    if (ack == CAN_OK)
    {
      countAck++;
      Serial.println(F("Message Sent Successfully!"));
    }
    else
    {
      Serial.print(F("Error Sending Message...ACK-"));
      Serial.println(ack);
    }
    delay(message.timeInterval);
  }
  delay(100);
  if (countAck == message.totalFrame)
  {
    Serial.println(strAck);
  }
  else
  {
    Serial.println(F("ackerror"));
    delay(100);
  }
}

// buzzer sound @param:buzzer time
void buzzerSound(uint8_t delayTime){
  tone(BUZZER, BUZZERFREQ); // Send 1KHz sound signal...
  delay(delayTime);         // ...for 0.5 sec
  noTone(BUZZER);           // Stop sound...
}
