#include "espnow/espnow.h"
#include "neopixel/leds.h"   // 👈 aquí cridem les funcions dels LEDs
#include <RTClib.h>

extern String debugMsg;
extern String debugMsg2;
extern bool reescriure;
extern uint8_t lastBri0;
extern uint8_t lastBri1;
extern uint8_t bri0;
extern uint8_t bri1;
extern uint8_t targetBri0;
extern uint8_t targetBri1;
extern uint8_t briSteps;
extern uint8_t controladorAdress[];
extern RTC_DS3231 rtc;
extern bool alarmActive;
extern uint8_t alarmSavedBri0;
extern uint8_t alarmSavedBri1;

uint8_t remotePreset0 = 2;
uint8_t remotePreset1 = 2;

namespace {
constexpr size_t MAX_ESPNOW_MESSAGE = 64;

struct EspNowMessage {
    char text[MAX_ESPNOW_MESSAGE];
};

QueueHandle_t messageQueue = nullptr;
}

void sendMessage(const uint8_t *mac, const char *msg) {
    esp_now_send(mac, (const uint8_t *)msg, strlen(msg) + 1);
    Serial.printf("📤 ESP-NOW enviat: %s\n", msg);
}

void sendLedState() {
    char state[80];
    DateTime now = rtc.now();
    snprintf(state, sizeof(state), "STATE,%u,%u,%u,%u,%u,%u,%u,%u,%02u:%02u",
             ledStrips[0].targetBrightness, ledStrips[0].preset,
             ledStrips[1].targetBrightness, ledStrips[1].preset,
             bri0, remotePreset0, bri1, remotePreset1,
             now.hour(), now.minute());
    esp_now_send(controladorAdress, (const uint8_t *)state, strlen(state) + 1);
}

void setupEspNowReceiver() {
    messageQueue = xQueueCreate(16, sizeof(EspNowMessage));
    if (messageQueue == nullptr) {
        Serial.println("ERROR creant la cua d'ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(onDataRecv);
}

// ===============================
// RX CALLBACK
// ===============================
void onDataRecv(const uint8_t *mac,
                      const uint8_t *incomingData,
                      int len) {

    if (messageQueue == nullptr || incomingData == nullptr || len <= 0) {
        return;
    }

    EspNowMessage message = {};
    size_t copyLength = min(static_cast<size_t>(len), MAX_ESPNOW_MESSAGE - 1);
    memcpy(message.text, incomingData, copyLength);
    message.text[copyLength] = '\0';

    xQueueSend(messageQueue, &message, 0);
}

void processEspNowMessages() {
    if (messageQueue == nullptr) {
        return;
    }

    EspNowMessage incomingMessage;
    while (xQueueReceive(messageQueue, &incomingMessage, 0) == pdTRUE) {
        String msg = incomingMessage.text;
        msg.trim();

        if (msg == "ALARM_OFF" || msg == "ALARM_STOPPED") {
            stopAlarm();
            continue;
        }

        if (msg.startsWith("STATE,")) {
            int values[8];
            if (sscanf(msg.c_str(), "STATE,%d,%d,%d,%d,%d,%d,%d,%d",
                       &values[0], &values[1], &values[2], &values[3],
                       &values[4], &values[5], &values[6], &values[7]) == 8) {
                bri0 = constrain(values[4], 0, 255);
                remotePreset0 = constrain(values[5], 1, 4);
                bri1 = constrain(values[6], 0, 255);
                remotePreset1 = constrain(values[7], 1, 4);
            }
            continue;
        }

        Serial.printf("📩 ESP-NOW rebut: %s\n", msg.c_str());
        notifyLCDActivity();

        bool isBrightnessCommand =
            msg.startsWith("+bri") ||
            msg.startsWith("-bri") ||
            msg.startsWith("bri");

        if (msg.startsWith("setParet=")) {
            ledStrips[0].targetBrightness = constrain(msg.substring(9).toInt(), 0, 255);
            reescriure = true;
            sendLedState();
            continue;
        }
        if (msg.startsWith("setPrestatge=")) {
            ledStrips[1].targetBrightness = constrain(msg.substring(13).toInt(), 0, 255);
            reescriure = true;
            sendLedState();
            continue;
        }

        // ===============================
        // COMANDES GLOBALS
        // ===============================
        if (msg == "toggleAll") {
            toggleParet();
            togglePrestatge();
            esp_now_send(controladorAdress, (const uint8_t*)"toggleTauleta", strlen("toggleTauleta") + 1);
            esp_now_send(controladorAdress, (const uint8_t*)"toggleGeneral", strlen("toggleGeneral") + 1);
            debugMsg2 = "Rebut toggleAll...";
        }
    else if (msg == "+briAll") {
        briPlusParet();
        briPlusPrestatge();
        esp_now_send(controladorAdress, (const uint8_t*)"+briTauleta", strlen("+briTauleta") + 1);
        esp_now_send(controladorAdress, (const uint8_t*)"+briGeneral", strlen("+briGeneral") + 1);
        debugMsg2 = "Rebut +briAll...";
    }
    else if (msg == "-briAll") {
        briMinusParet();
        briMinusPrestatge();
        esp_now_send(controladorAdress, (const uint8_t*)"-briTauleta", strlen("-briTauleta") + 1);
        esp_now_send(controladorAdress, (const uint8_t*)"-briGeneral", strlen("-briGeneral") + 1);
        debugMsg2 = "Rebut -briAll...";
    }
    else if (msg == "presetAll") {
        presetParet();
        presetPrestatge();
        debugMsg2 = "Rebut presetAll...";
    }

    // ===============================
    // TIRA 0 (PARET)
    // ===============================
    else if (msg == "toggleParet") {
        toggleParet();
        debugMsg2 = "Rebut toggleParet...";
    }
    else if (msg == "+briParet") {
        briPlusParet();
        debugMsg2 = "Rebut +briParet...";
    }
    else if (msg == "-briParet") {
        briMinusParet();
        debugMsg2 = "Rebut -briParet...";
    }
    else if (msg == "presetParet") {
        presetParet();
        debugMsg2 = "Rebut presetParet...";
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
        if (!isBrightnessCommand) {
            sendLedState();
        }
    }
}

