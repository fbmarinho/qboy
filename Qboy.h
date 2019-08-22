/*
  Qboy.h - Library for Qboy IOT Device
  Created by Felipe B. Marinho, August 12, 2019.
  All rights reserved.
*/

#ifndef Qboy_h
#define Qboy_h

#include "Arduino.h"

class Qboy
{
public:
  Qboy(const char *id);
  void connect(const char *ssid, const char *password, int _pingInterval, int _pairInterval, int _listenInterval);
  void loop();

private:
  const char *_id;
  static void _tick(Qboy *pThis);
  void tick();
  void ping();
  void pair();
  void listen();
  void call(const char *m1, unsigned int m1_time, unsigned int m1_count, const char *m2, unsigned int m2_time, unsigned int m2_count, bool _vibration);
  void showLogo();
  void backlightOn();
  void backlightOff();
  void vibrationOn();
  void vibrationOff();
  const char *idPedido;
  bool isConnected;
  bool isPaired;
  int counterPing;
  int pingInterval;
  bool shouldPing;
  int counterPair;
  int pairInterval;
  bool shouldPair;
  int counterListen;
  int listenInterval;
  bool shouldListen;
  void delay_s(unsigned int time);
};

#endif