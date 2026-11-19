#include <IA.h>
#include <SoftwareSerial.h>
//#include "PMS.h"
#include <MHZ19.h>

#include <TinyWireSlave.h>

// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 1 )
#endif

volatile byte sent = 0;

#define PMS_RX 4   
#define PMS_TX 3   

//PMS::DATA data;

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data1;

volatile uint8_t parameter = 0;

//NeoSWSerial softwareSerial(RX, TX);
SoftwareSerial softwareSerialPMS(PMS_RX, PMS_TX);
//PMS pms(softwareSerial);
//MHZ19 mhz(&softwareSerialMHZ);

void receiveEvent(uint8_t c) {
    if (c < 1) {
      return;
    }
    if (c > TWI_RX_BUFFER_SIZE) {
     return;
    }
    parameter = TinyWireS.receive();
}

void requestEvent() {
  uint16_t result = 0;
  switch (parameter) {
    case PM_10_STANDARD:
      result = data1.pm10_standard;
      //result = 10;
      break;
    case PM_25_STANDARD:
      result = data1.pm25_standard;
      //result = 25;
      break;
    case PM_100_STANDARD:
      result = data1.pm100_standard;
      //result = 100;
      break;
    case PM_10_ENV:
      result = data1.pm10_env;
      //result = 10;
      break;
    case PM_25_ENV:
      result = data1.pm25_env;
      //result = 25;
      break;
    case PM_100_ENV:
      result = data1.pm100_env;
      //result = 100;
      break;
  }
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
}

void setup() {
  sent = 0;
  softwareSerialPMS.begin(9600);
  //pms.passiveMode();    
  TinyWireS.begin(I2C_PMS_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);
}

void loop() {
  /*pms.wakeUp();
  //tws_delay(3000);
  delay(3000);
  pms.requestRead();
  boolean flag = false;
  if (pms.readUntil(data)) {
    flag = true;
  } else {
    flag = false;
  }
  pms.sleep();
  //tws_delay(6000);
  delay(6000);*/
  if (readPMSdata(&softwareSerialPMS)) {
      
  }
  TinyWireS_stop_check();
}

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }  
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
  // put it into a nice struct :)
  memcpy((void *)&data1, (void *)buffer_u16, 30);
  if (sum != data1.checksum) {
    return false;
  }
  return true;
}
