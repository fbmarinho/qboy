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

//Constructor
Qboy::Qboy(const char *id)
{
  counterPing = 0;
  counterPair = 0;
  shouldPing = false;
  shouldPair = false;
  wifiStatus = "X";
  mainMessage = "Aguarde o chamado";
  _id = id;
}

// Setup stage
void Qboy::start()
{
  pinMode(D8, OUTPUT);
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
  delay(500);

  showLogo(2000);
}

void Qboy::scroll(char *message)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextWrap(false);
  display.setTextColor(WHITE);

  int x = display.width();
  int len = 12 * strlen(message); // 12 = 6 pixels/character * text size 2

  for (int i = 0; i <= 2 * len; i++)
  {
    display.clearDisplay();
    display.setCursor(x, 20);
    display.print(message);
    display.display();

    if (--x < -len)
      x = display.width();
  }
}

void Qboy::mainScreen()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.drawLine(0, 12, 128, 12, WHITE);

  display.setTextSize(1);

  display.setCursor(1, 1);
  display.print("WiFi: ");
  display.print(wifiStatus);

  display.setCursor(64, 1);
  display.print("Exame:");
  display.print(idPedido);

  display.setTextSize(2);

  display.setCursor(20, 20);
  display.print(mainMessage);

  display.display();
}

//Conectar a rede
void Qboy::connect(const char *ssid, const char *password, int _pingInterval, int _pairInterval, int _listenInterval)
{
  pingInterval = _pingInterval;
  pairInterval = _pairInterval;
  listenInterval = _listenInterval;

  isConnected = false;
  shouldPair = false;
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }
  Serial.print("Conectado no IP: ");
  Serial.println(WiFi.localIP());
  wifiStatus = "OK";
  isConnected = true;
  shouldPair = true;

  clock.attach(1, &Qboy::_tick, this); // Start clock tick
}

//Loop
void Qboy::loop()
{
  mainScreen();

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
    Serial.println("Ping !");

    wifiStatus = "OK";

    isConnected = true;
  }
  else
  {
    Serial.println("Not connected !");

    wifiStatus = "X";

    isConnected = false;
  }
}

//Parear dispositivo
void Qboy::pair()
{
  std::string url = std::string("http://easystop.com.br/api/qboy/dispositivo/parear/") + std::string(_id);

  //Serial.println(url.c_str());
  isPaired = false;
  HTTPClient http;
  http.begin(url.c_str());

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

      if (json != "[]")
      {

        deserializeJson(doc, json);

        JsonObject obj = doc[0];

        idPedido = obj["id_pedido_exame"];

        Serial.print("Pedido: ");
        Serial.println(idPedido);

        isPaired = true;
      }
      else
      {
        idPedido = " - ";
      }

      //const char *obj_id_Chamado = obj["id_Chamado"];
      //const char *obj_id_pedido_exame = obj["id_pedido_exame"];
      // const char *obj_id_dispositivo = obj["id_dispositivo"];
      // const char *obj_dt_associacao = obj["dt_associacao"];
      // const char *obj_is_Chamando = obj["is_Chamando"];
      // const char *obj_mensagem = obj["mensagem"];
      // const char *obj_segundos_mensagem = obj["segundos_mensagem"];
      // const char *obj_qtd_repeticoes_mensagem = obj["qtd_repeticoes_mensagem"];
      // const char *obj_mensagem1 = obj["mensagem1"];
      // const char *obj_segundos_mensagem1 = obj["segundos_mensagem1"];
      // const char *obj_qtd_repeticoes_mensagem1 = obj["qtd_repeticoes_mensagem1"];
      // const char *obj_qtd_vibracoes = obj["qtd_vibracoes"];
      // const char *obj_segundos_vibracoes = obj["segundos_vibracoes"];
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
  std::string url = std::string("http://easystop.com.br/api/qboy/dispositivo/checar/") + std::string(_id) + std::string("&") + std::string(idPedido);

  //Serial.println(url.c_str());
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
        //const char *obj_id_Chamado = obj["id_Chamado"];                             // "141"
        //const char *obj_id_pedido_exame = obj["id_pedido_exame"];                   // "1450"
        //const char *obj_id_dispositivo = obj["id_dispositivo"];                     // "1456600300kkkksss*"
        //const char *obj_dt_chamado = obj["dt_chamado"];                             // "2019-08-21 14:32:13"
        //const char *obj_is_Chamando = obj["is_Chamando"];                           // "0"
        const char *sala = obj["sala"]; // "07"

        const char *m1 = obj["mensagem"];                             // "Sala de massagem"
        unsigned int m1_time = atoi(obj["segundos_mensagem"]);        // "3"
        unsigned int m1_loops = atoi(obj["qtd_repeticoes_mensagem"]); // "2"

        const char *m2 = obj["mensagem1"];                             // "Teste de envio de mensagem1"
        unsigned int m2_time = atoi(obj["segundos_mensagem1"]);        // "3"
        unsigned int m2_loops = atoi(obj["qtd_repeticoes_mensagem1"]); // "2"

        bool _vibration = atoi(obj["qtd_vibracoes"]) > 0; // "3"
        //const char *obj_segundos_vibracoes = obj["segundos_vibracoes"];             // "1"
        //const char *obj_dt_chamado_frma = obj["dt_chamado_frma"];                   // "21-08-2019"
        //const char *obj_hr_chamado_formatado = obj["hr_chamado_formatado"];         // "14:32:13"
        mainMessage = "Dirija-se a sala";

        call(m1, m1_time, m1_loops, m2, m2_time, m2_loops, _vibration);
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

void Qboy::call(const char *m1, unsigned int m1_time, unsigned int m1_count, const char *m2, unsigned int m2_time, unsigned int m2_count, bool _vibration)
{
  while (m1_count > 0 || m2_count > 0)
  {
    if (m1_count > 0)
    {
      Serial.println(m1);
      if (_vibration)
      {
        vibrationOn();
      }
      delay_s(m1_time);
      m1_count--;
      vibrationOff();
    }
    if (m2_count > 0)
    {
      Serial.println(m2);
      if (_vibration)
      {
        vibrationOn();
      }
      delay_s(m2_time);
      m2_count--;
      vibrationOff();
    }
    delay(1000);
  }
}

void Qboy::vibrationOn()
{
  digitalWrite(D8, HIGH);
}

void Qboy::vibrationOff()
{
  digitalWrite(D8, LOW);
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