/*
  Qboy.cpp - Library for Qboy IOT Device
  Created by Felipe B. Marinho, August 12, 2019.
  All rights reserved.
*/

#include "Arduino.h"
#include <string.h>
#include "Qboy.h"

//For display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "brmed.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//For wifi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//Time control
#include <Ticker.h>
Ticker clock;
Ticker screenRefresh;
Ticker scroller;
Ticker blinker;

#define LED_PIN D6
#define VIBRATION_PIN D7
#define BEEP_PIN D8

//Constructor
Qboy::Qboy(String id)
{
  blinkCount = 0;
  counterPing = 0;
  counterPair = 0;
  shouldPing = false;
  shouldPair = false;
  wifiStatus = "X";
  idPedido = "WAIT";
  mainText = "AGUARDE";
  setMessage("Bem-vindo a BRMED");
  firsttime = false;
  _id = id;

  pinMode(LED_PIN, OUTPUT);
  pinMode(BEEP_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
}

// Setup stage
void Qboy::start()
{
  //initialize display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();

  showLogo(2000);
}

void Qboy::_refresh(Qboy *pThis)
{
  pThis->refresh();
}

void Qboy::refresh()
{
  shouldRefresh = true;
}

void Qboy::mainScreen()
{
  display.clearDisplay();

  display.drawLine(0, 12, 128, 12, WHITE);
  display.drawLine(0, 44, 128, 44, WHITE);

  display.setTextColor(WHITE);

  // First line
  display.setTextSize(1);
  display.setCursor(1, 1);
  display.print("WiFi: ");
  display.print(wifiStatus);

  display.setCursor(64, 1);
  display.print("Exame:");
  display.print(idPedido);

  // Main text
  display.setCursor(0, 18);
  display.setTextSize(3);
  display.print(mainText);

  // Lower line
  display.setCursor(0, 48);
  display.setTextSize(2);
  display.setTextWrap(false);
  display.print(message);

  scroll();

  display.display();
}

void Qboy::scroll()
{
  if (message.length() * 12 > display.width())
  {
    if (!firsttime)
    {
      message = String(message + ". ");
      firsttime = true;
    }

    String temp;
    String temp2;

    temp = message.substring(1, message.length());
    temp2 = message.substring(0, 1);

    message = String(temp + temp2);
  }
}

//Conectar a rede
void Qboy::connect(String ssid, String password, int _pingInterval, int _pairInterval, int _listenInterval)
{
  pingInterval = _pingInterval;
  pairInterval = _pairInterval;
  listenInterval = _listenInterval;

  isConnected = false;
  shouldPair = false;
  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(2);
    flash(500, 3);
  }

  Serial.print("CONECTADO");
  Serial.print(WiFi.localIP());

  wifiStatus = "OK";
  isConnected = true;
  shouldPair = true;

  clock.attach(1, &Qboy::_tick, this);                 // Start clock tick
  screenRefresh.attach_ms(100, &Qboy::_refresh, this); // Start screen refresh
}

//Loop
void Qboy::loop()
{
  if (shouldRefresh)
  {
    mainScreen();
    shouldRefresh = false;
  }

  if (!calling)
  {
    if (shouldPing)
    {
      ping();
      shouldPing = false;
    }

    if (isConnected && shouldPair)
    {
      pair();
      shouldPair = false;
    }

    if (isPaired && shouldListen)
    {
      listen();
      shouldListen = false;
    }
  }
}

void Qboy::_tick(Qboy *pThis)
{
  pThis->tick();
}

void Qboy::tick()
{
  if (counterPing >= pingInterval - 1)
  {
    shouldPing = true;
    counterPing = 0;
  }

  if (counterPair >= pairInterval - 1)
  {
    shouldPair = true;
    counterPair = 0;
  }

  if (counterListen >= listenInterval - 1)
  {
    shouldListen = true;
    counterListen = 0;
  }

  counterPing++;
  counterPair++;
  counterListen++;
}

void Qboy::ping()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    wifiStatus = "OK";
    isConnected = true;
  }
  else
  {
    wifiStatus = "X";
    mainText = "OFFLINE";
    setMessage("Verifique a conexao");
    isConnected = false;
  }
}

//Parear dispositivo
void Qboy::pair()
{
  isPaired = false;

  String url = String("http://easystop.com.br/api/qboy/dispositivo/parear/" + _id);

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();

  //Check the returning code
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {

      // Parsing
      const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(17) + 360;
      DynamicJsonDocument doc(capacity);

      String json = http.getString();

      DeserializationError err = deserializeJson(doc, json);

      //debug
      if (err)
      {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
      }

      if (json != "[]")
      {
        JsonObject obj = doc[0];

        idPedido = String(obj["id_pedido_exame"] | "ERRO");

        isPaired = true;
      }
      else
      {
        idPedido = " N/A";
        mainText = "ASSOCIE";
        setMessage("Dirija-se a recepcao");
        beep(1);
      }
    }
    else
    {
      Serial.print("HTTPCODE: ");
      Serial.println(httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); //Close connection
}

//Escutar chamados
void Qboy::listen()
{
  String url = String("http://easystop.com.br/api/qboy/dispositivo/checar/" + _id + "&" + idPedido);

  HTTPClient http;
  http.begin(url.c_str());

  int httpCode = http.GET();

  //Check the returning code
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {

      String json = http.getString();

      if (json != "[]")
      {
        // Parsing
        const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(21) + 520;
        DynamicJsonDocument doc(capacity);

        deserializeJson(doc, json);

        JsonObject obj = doc[0];

        calling = true;

        String room = String("SALA:" + String(obj["sala"] | "--"));
        String m1 = String(String(obj["mensagem"] | "msg 1"));
        String m2 = String(String(obj["mensagem1"] | "msg 2"));
        bool vibration = atoi(obj["qtd_vibracoes"]) > 0;
        unsigned int loops = atoi(obj["qtd_repeticoes_mensagem"]);

        //debug
        Serial.println(m1);
        Serial.println(m2);
        Serial.println(vibration);
        Serial.println(loops);

        call(room, m1, m2, vibration, loops);
      }
      else
      {
        mainText = "AGUARDE";
        setMessage("Fique atento ao chamado");
        calling = false;
      }
    }
    else
    {
      Serial.print("HTTPCODE: ");
      Serial.println(httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); //Close connection
}

void Qboy::call(String _room, String _m1, String _m2, bool _vibration, unsigned int _loops)
{
  mainText = _room;

  if (_vibration)
  {
    flash(1000, _loops);
  }

  while (_loops > 0)
  {

    setMessage(_m1);

    _loops--;
  }

  calling = false;
}

void Qboy::_blink(Qboy *pThis)
{
  pThis->blink();
}

void Qboy::blink()
{
  if (blinkCount > 0)
  {
    digitalWrite(LED_PIN, !(digitalRead(LED_PIN)));
    blinkCount--;
  }
  else
  {
    blinker.detach();
    blinkCount = 0;
    digitalWrite(LED_PIN, LOW);
  }
}

void Qboy::flash(uint32_t interval, unsigned int times)
{
  blinkCount = times;
  blinker.attach_ms(interval, &Qboy::_blink, this);
}

void Qboy::beep(unsigned int times)
{
  for (int i = 1; i <= times; i++)
  {
    digitalWrite(BEEP_PIN, HIGH);
    delay(100);
    digitalWrite(BEEP_PIN, LOW);
    delay(100);
  }
}

void Qboy::vibrate(unsigned int times, unsigned int interval)
{
  for (int i = 1; i <= times; i++)
  {
    digitalWrite(VIBRATION_PIN, HIGH);
    delay_s(interval);
    digitalWrite(VIBRATION_PIN, LOW);
    delay_s(interval);
  }
}

void Qboy::delay_s(unsigned int time)
{
  for (unsigned int i = 0; i < time; i++)
  {
    delay(1000);
  }
}

void Qboy::showLogo(int time)
{
  display.clearDisplay();
  display.drawBitmap(0, 0, logo, 128, 64, WHITE);
  display.display();
  delay(time);
}

void Qboy::setMessage(String _message)
{
  if (_message != message)
  {
    message = _message;
    firsttime = false;
  }
}