#include "espnow/espnow.h"
#include "neopixel/leds.h"   // 👈 aquí cridem les funcions dels LEDs

extern String debugMsg;
extern String debugMsg2;
extern uint8_t lastBri0;
extern uint8_t lastBri1;
extern uint8_t bri0;
extern uint8_t bri1;
extern uint8_t targetBri0;
extern uint8_t targetBri1;
extern uint8_t briSteps;

void sendMessage(const uint8_t *mac, const char *msg) {
    esp_now_send(mac, (const uint8_t *)msg, strlen(msg) + 1);
    Serial.printf("📤 ESP-NOW enviat: %s\n", msg);
}

// ===============================
// RX CALLBACK
// ===============================
void onDataRecv(const uint8_t *mac,
                      const uint8_t *incomingData,
                      int len) {

    // 🔴 IMPORTANT: assumim STRING només si acaba amb \0
    if (incomingData[len - 1] != '\0') {
        Serial.println("⚠️ Paquet no-string rebut (ignorat)");
        return;
    }

    String msg = String((char *)incomingData);
    msg.trim();

    Serial.printf("📩 ESP-NOW rebut: %s\n", msg.c_str());

    // ===============================
    // COMANDES GLOBALS
    // ===============================
    if (msg == "toggleAll") {
        toggleTauleta();
        togglePrestatge();
        debugMsg2 = "Rebut toggleAll...";
    }
    else if (msg == "+briAll") {
        briPlusTauleta();
        briPlusPrestatge();
        debugMsg2 = "Rebut +briAll...";
    }
    else if (msg == "-briAll") {
        briMinusTauleta();
        briMinusPrestatge();
        debugMsg2 = "Rebut -briAll...";
    }
    else if (msg == "presetAll") {
        presetTauleta();
        presetPrestatge();
        debugMsg2 = "Rebut presetAll...";
    }

    // ===============================
    // TIRA 0 (TAULETA)
    // ===============================
    else if (msg == "toggleDespatx") {
        toggleTauleta();
        debugMsg2 = "Rebut toggleTauleta...";
    }
    else if (msg == "+briDespatx") {
        briPlusTauleta();
        debugMsg2 = "Rebut +briTauleta...";
    }
    else if (msg == "-briDespatx") {
        briMinusTauleta();
        debugMsg2 = "Rebut -briTauleta...";
    }
    else if (msg == "presetDespatx") {
        presetTauleta();
        debugMsg2 = "Rebut presetTauleta...";
    }

    // ===============================
    // TIRA 1 (PRESTATGE)
    // ===============================
    else if (msg == "togglePrestatge") {
        togglePrestatge();
        debugMsg2 = "Rebut togglePrestatge...";
    }
    else if (msg == "+briPrestatge") {
        briPlusPrestatge();
        debugMsg2 = "Rebut +briPrestatge...";
    }
    else if (msg == "-briPrestatge") {
        briMinusPrestatge();
        debugMsg2 = "Rebut -briPrestatge...";
    }
    else if (msg == "presetPrestatge") {
        presetPrestatge();
        debugMsg2 = "Rebut presetPrestatge...";
    }
    else if (msg.startsWith("briPrestatge=")) {
        String valueStr = msg.substring(strlen("briPrestatge="));
        int value = valueStr.toInt();  // convertir a número
        if (value >= 0 && value <= 255) {  // assegurar que està dins del rang de uint8_t
            bri0 = value;
            Serial.printf("ESPNOW: briPrestatge actualitzat a %d\n", bri0);
        } else {
            Serial.println("⚠️ Valor de briPrestatge fora de rang (0-255)");
        }
    }
    else if (msg.startsWith("briTauleta=")) {
        String valueStr = msg.substring(strlen("briTauleta="));
        int value = valueStr.toInt();  // convertir a número
        if (value >= 0 && value <= 255) {  // assegurar que està dins del rang de uint8_t
            bri1 = value;
            Serial.printf("ESPNOW: briTauleta actualitzat a %d\n", bri1);
        } else {
            Serial.println("⚠️ Valor de briTauleta fora de rang (0-255)");
        }
    }

    else {
        Serial.println("⚠️ Comanda desconeguda");
    }
}

