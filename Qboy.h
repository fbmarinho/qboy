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
  void connect(const char *ssid, const char *password, int checkInterval);

  int idPedido;
  bool isConnected;
  bool isPaired;

private:
  const char *_id;
  static void _ping(Qboy *pThis);
  void ping();
  void pair();
  void setConnectionState(bool state);
  void showLogo();
  void backlightOn();
  void backlightOff();
  void vibrate(int n, int interval);
};

#endif