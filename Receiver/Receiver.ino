#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>
#include <EEPROM.h>

#define SERVER_ADDRESS 1
#define CLIENT_ADDRESS 2

// Singleton instance of the radio driver
RH_NRF24 driver;

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

int deviceCountAddres = 0;

const int buttonPin = 3;
const int ledPin = 2;
int buttonState = 0;

void setup() 
{
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!manager.init())
    Serial.println("init failed");
}

// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void loop()
{
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    digitalWrite(ledPin, HIGH);
    setupNewDevice();
  }
  
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      float value = 0.0;
      memcpy(&value, buf, sizeof(float));
      
      Serial.print(from, HEX);
      Serial.print(":");
      Serial.println(value);
    }
  }
}

void setupNewDevice()
{
  delay(1000);
  uint8_t deviceCount = EEPROM.read(deviceCountAddres);
  Serial.println(deviceCount);
  if(deviceCount == 254)
    return;
  if (deviceCount == 255)
    deviceCount = 3;
  else
    deviceCount++;  
  uint8_t newAddress[] = {deviceCount};
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  
  manager.sendto(newAddress, sizeof(newAddress), CLIENT_ADDRESS);
  delay(1000);
  if (manager.available())
  {
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      if(from == 2)
      {
        EEPROM.update(deviceCountAddres, deviceCount);
        Serial.println("Successful setup!");
      }
      else
        Serial.println("Setting up new device failed 1!");
    }
    else
      Serial.println("Setting up new device failed 2!");
  }
  else
    Serial.println("Setting up new device failed 3!");    
    
  digitalWrite(ledPin, LOW);
}

