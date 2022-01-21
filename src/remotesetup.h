#ifndef REMOTESETUP_H
#define REMOTESETUP_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

int remoteSetup(const char* setup_ssid, const char* setup_password);

#endif