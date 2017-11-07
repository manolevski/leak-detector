#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1

RH_NRF24 driver;
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
  
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  manager.init();

  uint8_t deviceAddres = EEPROM.read(addressLocation);
  if (deviceAddres < 255)
    manager.setThisAddress(deviceAddres);
}

uint8_t data[sizeof(float)];
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void loop()
{
  buttonPress();

  if(setupMode)
    setupNewAddress();
  else
  {
    float voltage = analogRead(A0) * (5.0 / 1023.0);
    memcpy(data, &voltage, sizeof(voltage));
  
    manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS);
  
    delay(5000);
  }
}

void buttonPress()
{
  buttonState = digitalRead(buttonPin);
  int button_delay = 0;
  while (buttonState == HIGH){
    button_delay++;
    delay(100);
    buttonState = digitalRead(buttonPin);

    if(button_delay == 1) { //short press
      ledblink(2, 100, ledPin);
      setupMode = 1;
    }
    if(button_delay == 70){ //long press
      setupMode = 0;
      EEPROM.update(addressLocation, 255);
      manager.setThisAddress(2);
      ledblink(5, 500, ledPin);
    }        
  }
}

void setupNewAddress()
{
  digitalWrite(ledPin, HIGH);
  uint8_t setupCommand[] = "setup";
  if (manager.sendtoWait(setupCommand, sizeof(setupCommand), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      if(from == 1)
      {
        EEPROM.update(addressLocation, buf[0]);
        manager.setThisAddress(buf[0]);
        setupMode = 0;
        digitalWrite(ledPin, LOW);
      }
    }
  }
  delay(2000);
}

void ledblink(int times, int lengthms, int pinnum){
  for (int x=0; x<times;x++) {
    digitalWrite(pinnum, HIGH);
    delay (lengthms);
    digitalWrite(pinnum, LOW);
    delay(lengthms);
  }
}
