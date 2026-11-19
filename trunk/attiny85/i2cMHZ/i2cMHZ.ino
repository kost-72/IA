#include <IA.h>
#include <SoftwareSerial.h>
//#include <MHZ19.h>

#include <TinyWireSlave.h>

// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 1 )
#endif

volatile byte sent = 0;

#define MHZ_RX 4   
#define MHZ_TX 3   

const byte cmdMHZ[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

uint16_t ppmCO2;

volatile uint8_t parameter = 0;

volatile unsigned long previousMillis = 0;
const unsigned long period = 5000;   

SoftwareSerial softwareSerialMHZ(MHZ_RX, MHZ_TX);
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
    case PPM_CO2:
      result = ppmCO2;
      //result = 211;
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
  softwareSerialMHZ.begin(9600); 
  TinyWireS.begin(I2C_MHZ_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > period) {
    previousMillis = currentMillis;   
    if (readMHZdata() ) {
      
    }
    /*MHZ19_RESULT response = mhz.retrieveData();
    if (response == MHZ19_RESULT_OK)  {
      ppmCO2 = mhz.getCO2();  
    } else {
      ppmCO2 = 1;  
    }*/
  }  
  TinyWireS_stop_check();
}

boolean readMHZdata() {
  unsigned char response[9];
  softwareSerialMHZ.write(cmdMHZ, 9);
  memset(response, 0, 9);
  softwareSerialMHZ.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) {
    crc += response[i];
  }
  crc = 255 - crc;
  crc++;
  if ((response[0] == 0xFF) && (response[1] == 0x86) && (response[8] == crc)) {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    ppmCO2 = (256*responseHigh) + responseLow;
    return true;
  } else {
    return false;    
  }
}
