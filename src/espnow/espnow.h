#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>



// Enviar missatge de text (compatibilitat amb el que ja tens)
void sendMessage(const uint8_t *mac, const char *msg);

// Notify the main loop that a received message is user-visible activity.
void notifyLCDActivity();

// Callback RX (NO cridar directament)
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

#endif
