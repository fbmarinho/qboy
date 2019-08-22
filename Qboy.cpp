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
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "logo.h"

//For wifi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//Time controle
#include <Ticker.h>
Ticker clock;
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
Qboy::Qboy(const char *id)
{
  //setup
  counterPing = 0;
  counterPair = 0;
  shouldPing = false;
  shouldPair = false;
  pinMode(BL_PIN, OUTPUT);
  _id = id;
  showLogo();
  Serial.println("QBoy V1.0");
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
  isConnected = true;
  shouldPair = true;

  clock.attach(1, &Qboy::_tick, this); // Start clock tick
}

//Loop
void Qboy::loop()
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
    isConnected = true;
  }
  else
  {
    isConnected = false;
    Serial.println("Not connected !");
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

void Qboy::backlightOn()
{
  digitalWrite(BL_PIN, HIGH);
}

void Qboy::backlightOff()
{
  digitalWrite(BL_PIN, LOW);
}

void Qboy::vibrationOn()
{
  digitalWrite(D8, HIGH);
}

void Qboy::vibrationOff()
{
  digitalWrite(D8, LOW);
}

void Qboy::showLogo()
{
  backlightOn();
  screen.begin();
  screen.setTextSize(1);
  screen.setTextColor(BLACK);
  screen.clearDisplay();
  screen.setCursor(0, 0);
  screen.drawBitmap(0, 0, logo, 84, 48, BLACK);
  screen.display();
}

void Qboy::delay_s(unsigned int time)
{
  for (unsigned int i = 0; i < time; i++)
  {
    delay(1000);
  }
}