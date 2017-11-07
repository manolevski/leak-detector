#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1

// Singleton instance of the radio driver
RH_NRF24 driver;

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

int addressLocation = 0;

const int buttonPin = 3;
const int ledPin = 2;
int buttonState = 0;
int setupMode = 0;

void setup() 
{
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  //Serial.begin(9600);
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  manager.init();

  uint8_t deviceAddres = EEPROM.read(addressLocation);
  if (deviceAddres < 255)
    manager.setThisAddress(deviceAddres);
}

uint8_t data[sizeof(float)];

void loop()
{
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    digitalWrite(ledPin, HIGH);
    setupMode = 1;
  }

  if(setupMode)
    setupNewAddress();
  else
  {
    float voltage = analogRead(A0) * (5.0 / 1023.0);
  
    memcpy(data, &voltage, sizeof(voltage));
  
    // Send a message to manager_server
    manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS);
  
    delay(5000);
  }
}

void setupNewAddress()
{
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      if(from == 1)
      {
        uint8_t data[] = "success";
        manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS);
        EEPROM.update(addressLocation, buf);
        setupMode = 0;
        softwareReset(WDTO_60MS);
      }        
    }
  }
}

void softwareReset( uint8_t prescaller) {
  wdt_enable( prescaller);
  while(1) {}
}

