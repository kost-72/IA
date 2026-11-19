#include <Servo.h>
#include "ServoEaser.h"
#include <EEPROM.h>
//#include <TinyWireSlave.h>

/*#ifndef TWI_RX_BUFFER_SIZE
  #define TWI_RX_BUFFER_SIZE ( 1 )
#endif*/

#define PIN_SERVO 1 // 0
#define PIN_CONTROL 3 // 1
#define PIN_OPEN_LED 0 // 3
#define PIN_CLOSE_LED 4 // 4

#define DOOR_SERVO_DEGREES_CORRECTION 7
#define MSEC_90_DEGREE 3500

int servoFrameMillis = 20;  // minimum time between servo updates

volatile byte angle;
volatile byte anglePrevious;

//volatile byte sent = 0;

Servo servo1;
ServoEaser servoEaser;

/*void receiveEvent(uint8_t c) {
    if (c < 1) {
      return;
    }
    if (c > TWI_RX_BUFFER_SIZE) {
     return;
    }
    angle = TinyWireS.receive();
}

void requestEvent() {
  uint16_t result = angle;
  sent = sent + 1; 
  byte b;
  switch (sent) {
    case 1:
      b = result & 0xFF;
      break;
    case 2:
      b = result >> 8;
      sent = 0;
      break;
  }
  TinyWireS.send(b);
}*/

void setup() {
  pinMode(PIN_OPEN_LED, OUTPUT);
  pinMode(PIN_CLOSE_LED, OUTPUT);
  pinMode(PIN_CONTROL, INPUT);
  angle = EEPROMReadByte(0);
  if (angle < 0) {
    angle = 0;
  } else if (angle > 90) {
    angle = 90;   
  }
  anglePrevious = angle;
  doLed();  
  //sent = 0;
  //TinyWireS.begin(6);
  //TinyWireS.onReceive(receiveEvent);
  //TinyWireS.onRequest(requestEvent);
  servo1.write(angle);  
  servo1.attach(PIN_SERVO);
  servoEaser.begin(servo1, servoFrameMillis); 
}

void loop() {
  int value = digitalRead(PIN_CONTROL);
  if (value == LOW) {
    angle = 0;
  } else {
    angle = 90;
  } 
  if (angle != anglePrevious) {   
    EEPROMUpdateByte(0,angle);
  }
  servoEaser.update();
  if (servoEaser.hasArrived()) { 
    doLed();
    if (angle != anglePrevious) {   
      int duration = MSEC_90_DEGREE * ((float) abs(angle - anglePrevious)/90);
      servoEaser.easeTo(angle + DOOR_SERVO_DEGREES_CORRECTION, duration);
      anglePrevious = angle;
    }
  }
  //TinyWireS_stop_check();
}

void EEPROMWriteByte(int address, byte value) {
  EEPROM.write(address, value);
}

void EEPROMUpdateByte(int address, byte value) {
  EEPROM.update(address, value);
}

byte EEPROMReadByte(int address) {
  return EEPROM.read(address);
}

void doLed() {
  if (angle != anglePrevious) {   
    if (angle == 0) {
      digitalWrite(PIN_CLOSE_LED, HIGH);   
    } else {
      digitalWrite(PIN_OPEN_LED, HIGH);
    }
  } else {
    if (angle == 0) {
      digitalWrite(PIN_CLOSE_LED, HIGH);   
      digitalWrite(PIN_OPEN_LED, LOW);  
    } else {
      digitalWrite(PIN_OPEN_LED, HIGH);
      digitalWrite(PIN_CLOSE_LED, LOW);  
    }  
  }
}
