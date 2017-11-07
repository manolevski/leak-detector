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
int setupMode = 0;

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
  buttonPress();
  
  if(setupMode)
    setupNewDevice();
  else
  {
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
}

void buttonPress(){
  buttonState = digitalRead(buttonPin);
  int button_delay = 0;
  while (buttonState == HIGH){
    button_delay++;
    delay(100);
    buttonState = digitalRead(buttonPin);

    if(button_delay == 10) { //short press
      ledblink(2, 100, ledPin);
      setupMode = 1;
    }
    if(button_delay == 100){ //long press
      setupMode = 0;
      EEPROM.update(deviceCountAddres, 255);
      ledblink(5, 500, ledPin);
    }        
  }
}

void setupNewDevice()
{
  digitalWrite(ledPin, HIGH);
  if (manager.available())
  {
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      if(from == 2 && strcmp(buf, "setup") == 0)
      {
        uint8_t deviceCount = EEPROM.read(deviceCountAddres);
        if(deviceCount == 254)
          return;
        if (deviceCount == 255)
          deviceCount = 3;
        else
          deviceCount++;  
        uint8_t newAddress[] = {deviceCount};
        uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
      
        if (manager.sendtoWait(newAddress, sizeof(newAddress), from))
        {
          EEPROM.update(deviceCountAddres, deviceCount);
          Serial.println("Successful setup!");
          setupMode = 0;
          digitalWrite(ledPin, LOW);
        }
        else
          Serial.println("Setting up new device failed!");
      }
    }
  }
}

void ledblink(int times, int lengthms, int pinnum){
  for (int x=0; x<times;x++) {
    digitalWrite(pinnum, HIGH);
    delay (lengthms);
    digitalWrite(pinnum, LOW);
    delay(lengthms);
  }
}
