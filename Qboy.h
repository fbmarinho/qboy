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
  void start();
  void loop();

private:
  const char *_id;
  static void _tick(Qboy *pThis);
  void tick();
  static void _refresh(Qboy *pThis);
  void refresh();
  bool shouldRefresh;
  void ping();
  void pair();
  void listen();
  void call(const char *m1, unsigned int m1_time, unsigned int m1_count, const char *m2, unsigned int m2_time, unsigned int m2_count, bool _vibration);
  void beep(unsigned int times);
  void flash(unsigned int times);
  void vibrate(unsigned int times, unsigned int interval);
  bool calling;
  char *sala;
  char *idPedido;
  char *wifiStatus;
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
  void showLogo(int time);
  void scroll(char *message);
  void mainScreen();
  char *mainText;
  char *message;
};

#endif