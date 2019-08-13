/*
  Qboy.cpp - Library for Qboy IOT Device
  Created by Felipe B. Marinho, August 12, 2019.
  All rights reserved.
*/

#include "Arduino.h"
#include "string.h"
#include "Qboy.h"

//For display
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "logo.h"

//For wifi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// 1      2      3      4      5      6         7     8  
// VCC / GND  / SCE  / RST /  DC / DN(MOSI) / SCLK / LED
// pins
const int8_t RST_PIN = D2;
const int8_t CE_PIN = D1;
const int8_t DC_PIN = D6;
//const int8_t DIN_PIN = D7;  // Uncomment for Software SPI
//const int8_t CLK_PIN = D5;  // Uncomment for Software SPI
const int8_t BL_PIN = D0;

Adafruit_PCD8544 screen = Adafruit_PCD8544(DC_PIN, CE_PIN, RST_PIN);

//Constructor
Qboy::Qboy(const char* id)
{
    //setup
    pinMode(BL_PIN, OUTPUT);
    _id = id;
    showLogo();
}

//Conectar a rede
void Qboy::connect(const char* ssid, const char* password)
{
  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {
    screen.drawLine(40,83,45,83, 0x0000);
    delay(200);
    screen.display();
    screen.drawLine(43,80,43,85, 0x0000);
    delay(200);
    screen.display();
  }

  screen.clearDisplay();
  screen.setCursor(17,20);
  screen.print("CONECTADO");
  screen.display();
  delay(2000);

  pair();
  
}


//Parear dispositivo
void Qboy::pair()
{
    std::string url = std::string("easystop.com.br/api/qboy/dispositivo/parear/") + std::string(_id);
    
    HTTPClient http;  //Object of class HTTPClient

    http.begin(url.c_str());

    screen.clearDisplay();
    screen.setCursor(0,0);
    screen.print(url.c_str());
    screen.display();


    int httpCode = http.GET();
    //Check the returning code                                                                  
    if (httpCode > 0) {

        screen.clearDisplay();
        screen.setCursor(20,20);
        screen.print(httpCode);
        screen.display();
        vibrate(2,1000);

    } else {

      backlightOff();

    }
     
    http.end();   //Close connection

}


//Escutar chamados
void Qboy::listen()
{
    
    
}


// Private methods
void Qboy::backlightOn()
{
    digitalWrite(BL_PIN, HIGH);
}

void Qboy::backlightOff()
{
    digitalWrite(BL_PIN, LOW);
}

void Qboy::vibrate(int n, int interval)
{
  for(int i = 0; i<= n; i++){
    digitalWrite(D8, HIGH);
    delay(interval);
    digitalWrite(D8, LOW);
    delay(interval);
  }
}

void Qboy::showLogo()
{
  backlightOn();
  screen.begin();
  screen.setTextSize(1);
  screen.setTextColor(BLACK);
  screen.clearDisplay();
  screen.setCursor(0,0);
  screen.drawBitmap(0, 0,  logo, 84, 48, BLACK);
  screen.display();
}
