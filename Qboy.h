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
    Qboy(const char* id);
    void connect(const char* ssid, const char* password);
    void pair();
    void listen();
    bool isConnected;
  private:
      const char* _id;
      void showLogo();
      void backlightOn();
      void backlightOff();
      void vibrate(int n, int interval);
};

#endif