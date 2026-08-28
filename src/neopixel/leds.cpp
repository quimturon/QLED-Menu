#include "leds.h"
#include "espnow/espnow.h"
#include <esp_now.h>

const int NUM_STRIPS = 4;
#define NUM_LEDS 69
#define PARET_START 0
#define PARET_END 32
#define PRESTATGE_START 33
#define PRESTATGE_END 68

extern bool reescriure;
extern uint8_t bri0;
extern uint8_t bri1;
extern uint8_t targetBri0;
extern uint8_t targetBri1;
extern uint8_t lastBri0;
extern uint8_t lastBri1;
extern int minBri;
extern int maxBri;
extern uint8_t briSteps;

LEDStrip ledStrips[NUM_STRIPS] = {
    {0, 0, 2, 50},
    {0, 0, 2, 50},
    {0, 0, 2, 50},
    {0, 0, 2, 50}
};

Adafruit_NeoPixel ledStrip(NUM_LEDS, 19, NEO_GRBW + NEO_KHZ800);

uint32_t scaleColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white,
                    uint8_t brightness) {
    return ledStrip.Color(
        (uint16_t)red * brightness / 255,
        (uint16_t)green * brightness / 255,
        (uint16_t)blue * brightness / 255,
        (uint16_t)white * brightness / 255
    );
}

void renderZone(int stripIndex, int firstLed, int lastLed, uint16_t hue) {
    uint8_t brightness = ledStrips[stripIndex].brightness;
    uint32_t color = 0;

    switch (ledStrips[stripIndex].preset) {
        case 1:
            for (int ledIndex = firstLed; ledIndex <= lastLed; ledIndex++) {
                color = ledStrip.ColorHSV(
                    (hue + (ledIndex - firstLed) * 65536 / (lastLed - firstLed + 1)) % 65536,
                    255,
                    brightness
                );
                ledStrip.setPixelColor(ledIndex, color);
            }
            return;
        case 3:
            color = scaleColor(255, 255, 255, 255, brightness);
            break;
        case 4:
            color = ledStrip.ColorHSV(hue % 65536, 255, brightness);
            break;
        case 2:
        default:
            color = scaleColor(100, 0, 0, 255, brightness);
            break;
    }

    for (int ledIndex = firstLed; ledIndex <= lastLed; ledIndex++) {
        ledStrip.setPixelColor(ledIndex, color);
    }
}

void setupLEDs() {
    ledStrip.begin();
    ledStrip.setBrightness(255);
    ledStrip.clear();
    ledStrip.show();

    for(int i=0;i<NUM_STRIPS;i++){
        ledStrips[i].brightness = 0;
        ledStrips[i].targetBrightness = 0;
        ledStrips[i].preset = 2;
    }
}

void LEDTask(void *pvParameters) {
    uint16_t hue = 0;
    while(true) {
        for(int s=0;s<NUM_STRIPS;s++){
            // Fading de brillantor
            if(ledStrips[s].brightness < ledStrips[s].targetBrightness) ledStrips[s].brightness++;
            else if(ledStrips[s].brightness > ledStrips[s].targetBrightness) ledStrips[s].brightness--;
        }
        renderZone(0, PARET_START, PARET_END, hue);
        renderZone(1, PRESTATGE_START, PRESTATGE_END, hue);
        ledStrip.show();
        hue += 256;
        if (ledStrips[0].brightness != ledStrips[0].targetBrightness) {vTaskDelay(5/portTICK_PERIOD_MS);}
        else {vTaskDelay(5/portTICK_PERIOD_MS);}
    }
}

// Funcions enviaBrillantor i onDataRecv: pots actualitzar-les per enviar/recebre info de totes les tiras
void enviaBrillantor(int stripIndex) {
    if(stripIndex < 0 || stripIndex >= NUM_STRIPS) return;
    sendLedState();
}

void toggleParet() {
    if(ledStrips[0].targetBrightness == 0) {
            ledStrips[0].targetBrightness = lastBri0;
    } else {
        lastBri0 = ledStrips[0].targetBrightness;
        ledStrips[0].targetBrightness = 0;
    }
    reescriure = true;
}
void togglePrestatge() {
    if(ledStrips[1].targetBrightness == 0) {
        ledStrips[1].targetBrightness = lastBri1;
    } else {
        lastBri1 = ledStrips[1].targetBrightness;
        ledStrips[1].targetBrightness = 0;
    }
    reescriure = true;
}
void briPlusParet() {
    ledStrips[0].targetBrightness = min(ledStrips[0].targetBrightness + briSteps, maxBri);
    reescriure = true;
}
void briMinusParet() {
    ledStrips[0].targetBrightness = max(ledStrips[0].targetBrightness - briSteps, 5);
    reescriure = true;
}
void briPlusPrestatge() {
    ledStrips[1].targetBrightness = min(ledStrips[1].targetBrightness + briSteps, maxBri);
    reescriure = true;
}
void briMinusPrestatge() {
    ledStrips[1].targetBrightness = max(ledStrips[1].targetBrightness - briSteps, 5);
    reescriure = true;
}
void presetParet() {
    ledStrips[0].preset += 1;
    if(ledStrips[0].preset > 4) ledStrips[0].preset = 1;
    reescriure = true;
}
void presetPrestatge() {
    ledStrips[1].preset += 1;
    if(ledStrips[1].preset > 4) ledStrips[1].preset = 1;
    reescriure = true;
}
