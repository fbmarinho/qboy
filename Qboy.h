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
  Qboy(String id);
  void connect(String ssid, String password, int _pingInterval, int _pairInterval, int _listenInterval);
  void start();
  void loop();

private:
  String _id;
  String wifiStatus;
  String idPedido;
  String mainText;
  String message;

  bool shouldRefresh;
  bool isConnected;
  bool isPaired;
  bool shouldPing;
  bool shouldPair;
  bool shouldListen;

  bool calling;
  bool firsttime;

  int counterPing;
  int pingInterval;
  int counterPair;
  int pairInterval;
  int counterListen;
  int listenInterval;

  static void _tick(Qboy *pThis);
  static void _refresh(Qboy *pThis);

  void tick();
  void refresh();

  void delay_s(unsigned int time);

  void mainScreen();
  void scroll(String _message);
  void ping();
  void pair();
  void listen();
  void call(String _room, String _m1, String _m2, bool _vibration, unsigned int _loops);
  void beep(unsigned int times);
  void flash(unsigned int times);
  void vibrate(unsigned int times, unsigned int interval);

  void showLogo(int time);
};

#endif