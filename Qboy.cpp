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

//Time controle
#include <Ticker.h>
Ticker pinger;
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

  pinMode(BL_PIN, OUTPUT);
  _id = id;
  showLogo();
}

//Conectar a rede
void Qboy::connect(const char *ssid, const char *password, int checkInterval)
{
  isConnected = false;
  WiFi.begin(ssid, password);
  pinger.attach(checkInterval, &Qboy::_ping, this);
}

void Qboy::_ping(Qboy *pThis)
{
  pThis->ping();
}

void Qboy::ping()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Ping !");
    isConnected = true;
    pair();
  }
}

//Parear dispositivo
void Qboy::pair()
{
  Serial.print("Chamando: ");

  std::string url = std::string("http://easystop.com.br/api/qboy/dispositivo/parear/") + std::string(_id);

  Serial.println(url.c_str());

  HTTPClient http;

  http.begin(url.c_str());

  int httpCode = http.GET();

  //Check the returning code
  if (httpCode > 0)
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
  }
  else
  {
    Serial.println("Erro ao parear !");
  }

  http.end(); //Close connection
}

// //Escutar chamados
// void Qboy::listen()
// {
// }

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
  for (int i = 0; i <= n; i++)
  {
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
  screen.setCursor(0, 0);
  screen.drawBitmap(0, 0, logo, 84, 48, BLACK);
  screen.display();
}
