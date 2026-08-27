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

        Serial.printf("📩 ESP-NOW rebut: %s\n", msg.c_str());
        notifyLCDActivity();

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
}

