// ================= LLIBRERIES DE SISTEMA =================
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>

// ================= LLIBRERIES WIFI I DADES =================
#include "esp_wifi.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <EEPROM.h>
#include <esp_now.h>

// ================= LLIBRERIES DE PANTALLES =================
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <LiquidCrystal_I2C.h>

// ================= FONTS OLED =================
#include <Fonts/FreeMonoBoldOblique24pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

// ================= LLIBRERIES DE LLUMS =================
#include <Adafruit_NeoPixel.h>

// ================= LLIBRERIES INPUTS =================
#include <AiEsp32RotaryEncoder.h>
#include <RTClib.h>

// ================= LLIBRERIES PRÒPIES =================
#include "ota/ota.h"
#include "wifi/wifi_manager.h"
#include "neopixel/leds.h"
#include "ntp/ntp.h"
#include "espnow/espnow.h"


// ========================================================
// ================= MENU SETTING =========================
// ========================================================

int menu = 0;
int menuIndex = 0;

DateTime lastUpdateOTA;
DateTime lastUpdateRTC;


// ========================================================
// ================= OLED =================================
// ========================================================

String debugMsg = "";
String debugMsg2 = "";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);


// ========================================================
// ================= WIFI =================================
// ========================================================

struct WiFiCred {
    const char* ssid;
    const char* pass;
};


// ========================================================
// ================= OTA ==================================
// ========================================================

String FW_VERSION;
String NEW_VERSION;

bool otaInProgress = false;
int needOTA = 0;

const char* releasesAPI =
    "https://api.github.com/repos/quimturon/QLED-Menu/releases/latest";

const char* firmwareURL =
    "https://github.com/quimturon/QLED-Menu/releases/latest/download/firmware.bin";


// ========================================================
// ================= EEPROM ===============================
// ========================================================

#define EEPROM_SIZE 160
#define SSID_ADDR 0
#define PASS_ADDR 64
#define VERSION_ADDR 128


// ========================================================
// ================= ESP-NOW ==============================
// ========================================================

uint8_t controladorAdress[] = {
    0x70,
    0x4B,
    0xCA,
    0x83,
    0x4A,
    0x20
};


// ========================================================
// ================= LED STRIPS ===========================
// ========================================================

uint8_t bri0;
uint8_t bri1;
uint8_t bri2;
uint8_t bri3;

uint8_t targetBri0;
uint8_t targetBri1;
uint8_t targetBri2;
uint8_t targetBri3;

uint8_t lastBri0;
uint8_t lastBri1;
uint8_t lastBri2;
uint8_t lastBri3;

int minBri = 5;
int maxBri = 255;

uint8_t briSteps = 50;

extern uint8_t remotePreset0;
extern uint8_t remotePreset1;

String nominalPreset[] = {
    "",
    "",
    "",
    ""
};


String callPreset(int stripIndex, int presetIndex) {

    if (presetIndex == 1) {
        nominalPreset[stripIndex] = "Rainbow";
        return "Rainbow";

    } else if (presetIndex == 2) {
        nominalPreset[stripIndex] = "Calid";
        return "Calid";

    } else if (presetIndex == 3) {
        nominalPreset[stripIndex] = "Blanc";
        return "Blanc";

    } else if (presetIndex == 4) {
        nominalPreset[stripIndex] = "Colorit";
        return "Colorit";

    } else {
        nominalPreset[stripIndex] = "Off";
        return "Off";
    }
}


// ========================================================
// ================= PIN DEFINITIONS ======================
// ========================================================

#define ENC1_A 34
#define ENC1_B 35

#define ENC2_A 36
#define ENC2_B 39

#define ENC3_A 32
#define ENC3_B 33

#define ENC4_A 25
#define ENC4_B 26

#define ENC5_A 14
#define ENC5_B 12

#define ENCODER_STEPS 4


// ========================================================
// ================= BOTONS ===============================
// ========================================================

#define BUTTON1 15
#define BUTTON2 27
#define BUTTON3 4
#define BUTTON4 5

// ES MANTÉ GPIO3
#define BUTTON5 3

#define ENC1_BTN 0
#define ENC2_BTN 1
#define ENC3_BTN 13
#define ENC4_BTN 23
#define ENC5_BTN -1


bool buttonState[] = {
    0,0,0,0,0,0,0,0,0,0
};

bool buttonState1 = 0;
bool buttonState2 = 0;
bool buttonState3 = 0;
bool buttonState4 = 0;
bool buttonState5 = 0;
bool buttonState6 = 0;
bool buttonState7 = 0;
bool buttonState8 = 0;
bool buttonState9 = 0;
bool buttonState10 = 0;


bool lastButtonState1 = HIGH;
bool lastButtonState2 = HIGH;
bool lastButtonState3 = HIGH;
bool lastButtonState4 = HIGH;
bool lastButtonState5 = HIGH;
bool lastButtonState6 = HIGH;
bool lastButtonState7 = HIGH;
bool lastButtonState8 = HIGH;
bool lastButtonState9 = HIGH;
bool lastButtonState10 = HIGH;


// ========================================================
// ================= ROTARY ENCODERS ======================
// ========================================================

AiEsp32RotaryEncoder enc1(
    ENC1_A,
    ENC1_B,
    ENC1_BTN,
    -1,
    ENCODER_STEPS
);

AiEsp32RotaryEncoder enc2(
    ENC2_A,
    ENC2_B,
    ENC2_BTN,
    -1,
    ENCODER_STEPS
);

AiEsp32RotaryEncoder enc3(
    ENC3_A,
    ENC3_B,
    ENC3_BTN,
    -1,
    ENCODER_STEPS
);

AiEsp32RotaryEncoder enc4(
    ENC4_A,
    ENC4_B,
    ENC4_BTN,
    -1,
    ENCODER_STEPS
);


// CORREGIT: A,B en lloc de B,A
AiEsp32RotaryEncoder enc5(
    ENC5_A,
    ENC5_B,
    ENC5_BTN,
    -1,
    ENCODER_STEPS
);


long encVal[5] = {
    0,0,0,0,0
};


// ========================================================
// ================= DISPLAYS =============================
// ========================================================

bool reescriure = false;

const unsigned long LCD_ACTIVE_TIMEOUT = 60000UL;
const unsigned long LCD_ALL_OFF_TIMEOUT = 10000UL;
bool lcdBacklightOn = true;
unsigned long lastLCDActivity = 0;
volatile bool lcdActivityPending = false;

LiquidCrystal_I2C lcd2004(0x27, 20, 4);
LiquidCrystal_I2C lcd1602(0x26, 16, 2);


// ========================================================
// ================= RTC ==================================
// ========================================================

RTC_DS3231 rtc;

int lastMinute = -1;


// ========================================================
// ================= FUNCTION PROTOTYPES ==================
// ========================================================

void updateLCD2004(int menu, int menuIndex);
void updateLCD1602(int menu, int menuIndex);
void updateOLED(char* buf);
void debugPrint(const String &msg);
void notifyLCDActivity();
void updateLCDBacklight();


// ========================================================
// ================= ISR ==================================
// ========================================================

void IRAM_ATTR readEncoder0() {
    enc1.readEncoder_ISR();
}

void IRAM_ATTR readEncoder1() {
    enc2.readEncoder_ISR();
}

void IRAM_ATTR readEncoder2() {
    enc3.readEncoder_ISR();
}

void IRAM_ATTR readEncoder3() {
    enc4.readEncoder_ISR();
}

void IRAM_ATTR readEncoder4() {
    enc5.readEncoder_ISR();
}


// ========================================================
// ================= LCD 2004 =============================
// ========================================================

void notifyLCDActivity() {
    lcdActivityPending = true;
}

void wakeLCDBacklight() {
    lcd2004.backlight();
    lcd1602.backlight();
    lcdBacklightOn = true;
    lastLCDActivity = millis();
}

bool areAllLightsOff() {
    for (int stripIndex = 0; stripIndex < NUM_STRIPS; stripIndex++) {
        if (ledStrips[stripIndex].targetBrightness != 0) {
            return false;
        }
    }

    return true;
}

void updateLCDBacklight() {
    if (lcdActivityPending) {
        lcdActivityPending = false;
        wakeLCDBacklight();
    }

    unsigned long timeout = areAllLightsOff()
        ? LCD_ALL_OFF_TIMEOUT
        : LCD_ACTIVE_TIMEOUT;

    if (
        lcdBacklightOn &&
        millis() - lastLCDActivity >= timeout
    ) {
        lcd2004.noBacklight();
        lcd1602.noBacklight();
        lcdBacklightOn = false;
    }
}

void updateLCD2004(int menu, int menuIndex) {
    lcd2004.clear();

    if (menu == 0) {

        lcd2004.setCursor(0,0);
        lcd2004.print("Firmware: ");
        lcd2004.print(FW_VERSION);

        lcd2004.setCursor(0,1);
        lcd2004.printf("MAC%s", WiFi.macAddress().c_str());

        if (needOTA == 1) {

            lcd2004.setCursor(0,2);
            lcd2004.print("Nova versio:");
            lcd2004.print(NEW_VERSION);

            lcd2004.setCursor(0,3);
            lcd2004.print("Actualitzant...");

        } else if (needOTA == 2) {

            lcd2004.setCursor(0,2);
            lcd2004.print("Tot actualitzat el:");

            lcd2004.setCursor(0,3);

            char buf[21];

            sprintf(
                buf,
                "%02d/%02d/%04d %02d:%02d",
                lastUpdateOTA.day(),
                lastUpdateOTA.month(),
                lastUpdateOTA.year(),
                lastUpdateOTA.hour(),
                lastUpdateOTA.minute()
            );

            lcd2004.print(buf);
        }

    } else if (menu == 1) {

        lcd2004.setCursor(0,0);
        lcd2004.printf(
            "%-9s%3d %s",
            "Paret",
            ledStrips[0].targetBrightness,
            callPreset(0, ledStrips[0].preset)
        );

        lcd2004.setCursor(0,1);
        lcd2004.printf(
            "%-9s%3d %s",
            "Prestatge",
            ledStrips[1].targetBrightness,
            callPreset(1, ledStrips[1].preset)
        );

        lcd2004.setCursor(0,2);
        lcd2004.printf(
            "%-9s%3d %s",
            "Tauleta",
            bri0,
            callPreset(2, remotePreset0)
        );

        lcd2004.setCursor(0,3);
        lcd2004.printf(
            "%-9s%3d %s",
            "General",
            bri1,
            callPreset(3, remotePreset1)
        );

    } else if (menu == 2) {

        lcd2004.setCursor(0,0);
        lcd2004.print("Hora RTC");

        DateTime now = rtc.now();

        char buf[21];

        sprintf(
            buf,
            "%02d/%02d/%04d",
            now.day(),
            now.month(),
            now.year()
        );

        lcd2004.setCursor(0,1);
        lcd2004.print(buf);

        sprintf(
            buf,
            "%02d:%02d:%02d",
            now.hour(),
            now.minute(),
            now.second()
        );

        lcd2004.setCursor(0,2);
        lcd2004.print(buf);
    }
}


// ========================================================
// ================= LCD 1602 =============================
// ========================================================

void updateLCD1602(int menu, int menuIndex) {
    lcd1602.clear();

    if (menu == 0) {

        lcd1602.setCursor(4,0);
        lcd1602.print("Firmware");

    } else if (menu == 1) {

        lcd1602.setCursor(5,0);
        lcd1602.print("Llums");

    } else if (menu == 2) {

        lcd1602.setCursor(6,0);
        lcd1602.print("RTC");
    }
}


// ========================================================
// ================= OLED =================================
// ========================================================

void updateOLED(char* buf) {

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setFont(&FreeSans18pt7b);

    display.setCursor(
        16,
        display.height()/2 + 14
    );

    display.print(buf);

    display.setCursor(0,10);

    display.setFont();

    display.println(debugMsg);
    display.println(debugMsg2);

    display.display();
}


// ========================================================
// ================= DEBUG ================================
// ========================================================

void debugPrint(const String &msg) {

    notifyLCDActivity();
    Serial.println(msg);

    display.setCursor(
        0,
        SCREEN_HEIGHT - 8
    );

    display.fillRect(
        0,
        SCREEN_HEIGHT - 8,
        SCREEN_WIDTH,
        8,
        SSD1306_BLACK
    );

    display.print(msg);
    display.display();
}


// ========================================================
// ================= SETUP ================================
// ========================================================

void setup() {

    Serial.begin(115200);

    Serial.println();
    Serial.println("=================================");
    Serial.println("       INICIANT ESP32");
    Serial.println("=================================");


    // ----------------------------------------------------
    // EEPROM / Firmware
    // ----------------------------------------------------

    FW_VERSION = readVersion();

    Serial.print("Versio llegida EEPROM: ");
    Serial.println(FW_VERSION);


    // ----------------------------------------------------
    // OLED
    // ----------------------------------------------------

    display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
    );

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.setTextSize(1);

    display.println("Iniciant ESP32...");

    display.setCursor(0,20);

    display.print("V");
    display.println(FW_VERSION);

    display.display();


    // ----------------------------------------------------
    // WIFI
    // ----------------------------------------------------

    if (!setup_wifi()) {

        display.clearDisplay();

        display.setCursor(0,0);
        display.println("ERROR WIFI");

        display.display();

        delay(5000);

        ESP.restart();
    }


    // ----------------------------------------------------
    // ESP-NOW
    // ----------------------------------------------------

    if (esp_now_init() != ESP_OK) {

        Serial.println("ERROR inicialitzant ESP-NOW");

        display.println("ESP-NOW ERROR");
        display.display();

        while (true) {
            delay(100);
        }
    }


    esp_now_peer_info_t peerInfo = {};

    memcpy(
        peerInfo.peer_addr,
        controladorAdress,
        6
    );

    peerInfo.channel = 0;
    peerInfo.encrypt = false;


    esp_now_del_peer(
        controladorAdress
    );


    if (
        esp_now_add_peer(&peerInfo)
        != ESP_OK
    ) {

        Serial.println(
            "ERROR afegint peer"
        );

    } else {

        Serial.println(
            "Peer afegit correctament"
        );
    }


    setupEspNowReceiver();


    Serial.println(
        "ESP-NOW inicialitzat"
    );

    Serial.print("Canal: ");
    Serial.println(
        WiFi.channel()
    );


    // ----------------------------------------------------
    // OLED WIFI OK
    // ----------------------------------------------------

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0,0);

    display.dim(true);
    display.display();

    delay(100);

    display.dim(false);
    display.display();

    display.println("WIFI OK");
    display.display();


    // ----------------------------------------------------
    // LEDS
    // ----------------------------------------------------

    setupLEDs();

    targetBri0 = 0;
    targetBri1 = 0;

    bri0 = 0;
    bri1 = 0;


    xTaskCreatePinnedToCore(
        LEDTask,
        "LED Task",
        4000,
        NULL,
        1,
        NULL,
        0
    );


    Serial.println(
        "Tasca LED creada"
    );


    // ----------------------------------------------------
    // LCD
    // ----------------------------------------------------

    lcd2004.init();
    lcd1602.init();

    wakeLCDBacklight();


    // ----------------------------------------------------
    // OLED
    // ----------------------------------------------------

    if (
        display.begin(
            SSD1306_SWITCHCAPVCC,
            0x3C
        )
    ) {

        display.clearDisplay();
        display.display();
    }


    Serial.println(
        "Pantalles inicialitzades"
    );


    // ----------------------------------------------------
    // RTC
    // ----------------------------------------------------

    if (!rtc.begin()) {

        debugPrint(
            "No s'ha trobat el RTC!"
        );
    }


    ntpInit(
        "pool.ntp.org",
        3600,
        3600
    );


    Serial.println(
        "Sincronitzat RTC amb NTP"
    );


    // ----------------------------------------------------
    // ENCODERS 3 I 4
    // ----------------------------------------------------

    pinMode(
        ENC3_A,
        INPUT_PULLUP
    );

    pinMode(
        ENC3_B,
        INPUT_PULLUP
    );

    pinMode(
        ENC4_A,
        INPUT_PULLUP
    );

    pinMode(
        ENC4_B,
        INPUT_PULLUP
    );


    // ----------------------------------------------------
    // ENCODERS
    // ----------------------------------------------------

    enc1.begin();
    enc1.setup(readEncoder0);
    enc1.setAcceleration(0);

    enc2.begin();
    enc2.setup(readEncoder1);
    enc2.setAcceleration(0);

    enc3.begin();
    enc3.setup(readEncoder2);
    enc3.setAcceleration(0);

    enc4.begin();
    enc4.setup(readEncoder3);
    enc4.setAcceleration(0);

    enc5.begin();
    enc5.setup(readEncoder4);
    enc5.setAcceleration(0);


    // ----------------------------------------------------
    // BOTONS
    // ----------------------------------------------------

    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUTTON3, INPUT_PULLUP);
    pinMode(BUTTON4, INPUT_PULLUP);

    // ES MANTÉ GPIO3
    pinMode(BUTTON5, INPUT_PULLUP);


    pinMode(ENC1_BTN, INPUT_PULLUP);
    pinMode(ENC2_BTN, INPUT_PULLUP);
    pinMode(ENC3_BTN, INPUT_PULLUP);
    pinMode(ENC4_BTN, INPUT_PULLUP);


    Serial.println(
        "Setup finalitzat"
    );

    Serial.println(
        "Menu inicial = 0 (Firmware)"
    );
}


// ========================================================
// ================= LOOP =================================
// ========================================================

void loop() {

    processEspNowMessages();

    static unsigned long lastStateSent = 0;
    if (millis() - lastStateSent >= 30000UL) {
        lastStateSent = millis();
        sendLedState();
    }

    ensureWiFi();


    // ====================================================
    // RTC
    // ====================================================

    DateTime now = rtc.now();

    char buf[9];

    snprintf(
        buf,
        sizeof(buf),
        "%02d:%02d",
        now.hour(),
        now.minute()
    );


    if (
        now.minute()
        != lastMinute
    ) {

        lastMinute =
            now.minute();

        reescriure = true;
    }


    // ====================================================
    // ENCODERS
    // ====================================================

    bool encoderMoved = false;


    // ----------------------------------------------------
    // ENCODER 1
    // ----------------------------------------------------

    if (enc1.encoderChanged()) {

        encoderMoved = true;

        encVal[0] =
            enc1.readEncoder();

        int delta =
            enc1.readEncoder();


        if (delta > 0) {

            ledStrips[0].targetBrightness =
                min(
                    ledStrips[0].targetBrightness
                    + briSteps,
                    maxBri
                );

        } else if (delta < 0) {

            ledStrips[0].targetBrightness =
                max(
                    ledStrips[0].targetBrightness
                    - briSteps,
                    minBri
                );
        }


        enviaBrillantor(0);

        enc1.reset();

        reescriure = true;
    }


    // ----------------------------------------------------
    // ENCODER 2
    // ----------------------------------------------------

    if (enc2.encoderChanged()) {

        encoderMoved = true;

        encVal[1] =
            enc2.readEncoder();

        int delta =
            enc2.readEncoder();


        if (delta > 0) {

            ledStrips[1].targetBrightness =
                min(
                    ledStrips[1].targetBrightness
                    + briSteps,
                    maxBri
                );

        } else if (delta < 0) {

            ledStrips[1].targetBrightness =
                max(
                    ledStrips[1].targetBrightness
                    - briSteps,
                    minBri
                );
        }


        enviaBrillantor(1);

        enc2.reset();

        reescriure = true;
    }


    // ----------------------------------------------------
    // ENCODER 3
    // ----------------------------------------------------

    if (enc3.encoderChanged()) {

        encVal[2] =
            enc3.readEncoder();

        encoderMoved = true;
    }


    // ----------------------------------------------------
    // ENCODER 4
    // ----------------------------------------------------

    if (enc4.encoderChanged()) {

        encVal[3] =
            enc4.readEncoder();

        encoderMoved = true;
    }


    // ----------------------------------------------------
    // ENCODER 5
    // ----------------------------------------------------

    if (enc5.encoderChanged()) {

        encVal[4] =
            enc5.readEncoder();

        encoderMoved = true;
    }


    if (encoderMoved) {

        reescriure = true;
    }


    // ====================================================
    // LECTURA DELS BOTONS
    // ====================================================

    buttonState1 = digitalRead(BUTTON1);
    buttonState2 = digitalRead(BUTTON2);
    buttonState3 = digitalRead(BUTTON3);
    buttonState4 = digitalRead(BUTTON4);

    // GPIO3
    buttonState5 = digitalRead(BUTTON5);

    buttonState6 = digitalRead(ENC1_BTN);
    buttonState7 = digitalRead(ENC2_BTN);
    buttonState8 = digitalRead(ENC3_BTN);
    buttonState9 = digitalRead(ENC4_BTN);

    // ENC5 no té botó
    buttonState10 = HIGH;

    bool buttonPressed =
        (lastButtonState1 == HIGH && buttonState1 == LOW) ||
        (lastButtonState2 == HIGH && buttonState2 == LOW) ||
        (lastButtonState3 == HIGH && buttonState3 == LOW) ||
        (lastButtonState4 == HIGH && buttonState4 == LOW) ||
        (lastButtonState5 == HIGH && buttonState5 == LOW) ||
        (lastButtonState6 == HIGH && buttonState6 == LOW) ||
        (lastButtonState7 == HIGH && buttonState7 == LOW) ||
        (lastButtonState8 == HIGH && buttonState8 == LOW) ||
        (lastButtonState9 == HIGH && buttonState9 == LOW);

    if (encoderMoved || buttonPressed) {
        notifyLCDActivity();
    }


    // ====================================================
    // BUTTON 1
    // ====================================================

    if (
        lastButtonState1 == HIGH &&
        buttonState1 == LOW
    ) {

        Serial.println("BUTTON 1 PREMUT");

        reescriure = true;


        if (menu == 0) {

            String newVersion;

            if (
                checkForUpdate(newVersion)
            ) {

                Serial.println(
                    "Nova versio disponible. Inici OTA..."
                );

                needOTA = 1;

                NEW_VERSION =
                    newVersion;

                updateLCD2004(
                    menu,
                    menuIndex
                );

                performOTA(
                    newVersion
                );

            } else {

                Serial.println(
                    "Tens la ultima versio."
                );

                lastUpdateOTA =
                    rtc.now();

                needOTA = 2;

                updateLCD2004(
                    menu,
                    menuIndex
                );
            }


        } else if (menu == 1) {

            if (
                ledStrips[0].targetBrightness
                > 0
            ) {

                lastBri0 =
                    ledStrips[0].targetBrightness;

                ledStrips[0].targetBrightness =
                    0;

            } else {

                ledStrips[0].targetBrightness =
                    lastBri0;
            }


            enviaBrillantor(0);


        } else if (menu == 2) {

            debugPrint(
                "Sincronitzant NTP..."
            );


            if (
                ntpSyncRTC(rtc)
            ) {

                lastUpdateRTC =
                    rtc.now();

                debugPrint(
                    "RTC actualitzat!"
                );

            } else {

                debugPrint(
                    "Error NTP"
                );
            }
        }
    }


    // ====================================================
    // BUTTON 2
    // ====================================================

    if (
        lastButtonState2 == HIGH &&
        buttonState2 == LOW
    ) {

        Serial.println("BUTTON 2 PREMUT");

        reescriure = true;


        if (menu == 0) {

            // Accio firmware


        } else if (menu == 1) {

            if (
                ledStrips[1].targetBrightness
                > 0
            ) {

                lastBri1 =
                    ledStrips[1].targetBrightness;

                ledStrips[1].targetBrightness =
                    0;

            } else {

                ledStrips[1].targetBrightness =
                    lastBri1;
            }


            enviaBrillantor(1);


        } else if (menu == 2) {

            // Accio RTC
        }
    }


    // ====================================================
    // BUTTON 3
    // ====================================================

    if (
        lastButtonState3 == HIGH &&
        buttonState3 == LOW
    ) {

        Serial.println("BUTTON 3 PREMUT");

        reescriure = true;


        if (menu == 0) {

            // Accio firmware


        } else if (menu == 1) {

            debugMsg =
                "Enviant togglePrestatge...";

            esp_now_send(
                controladorAdress,
                (uint8_t*)"togglePrestatge",
                strlen("togglePrestatge") + 1
            );


        } else if (menu == 2) {

            // Accio RTC
        }
    }


    // ====================================================
    // BUTTON 4
    // ====================================================

    if (
        lastButtonState4 == HIGH &&
        buttonState4 == LOW
    ) {

        Serial.println("BUTTON 4 PREMUT");

        reescriure = true;


        if (menu == 0) {

            // Accio firmware


        } else if (menu == 1) {

            debugMsg =
                "Enviant toggleTauleta...";

            esp_now_send(
                controladorAdress,
                (uint8_t*)"toggleTauleta",
                strlen("toggleTauleta") + 1
            );


        } else if (menu == 2) {

            // Accio RTC
        }
    }


    // ====================================================
    // BUTTON 5 - CANVI DE MENU
    // ====================================================

    if (
        lastButtonState5 == HIGH &&
        buttonState5 == LOW
    ) {

        Serial.println();
        Serial.println("========================");
        Serial.println("BUTTON 5 PREMUT");
        Serial.print("Menu anterior: ");
        Serial.println(menu);


        // Canvi de menu
        menu++;


        // Despres de RTC tornem a Firmware
        if (menu > 2) {
            menu = 0;
        }


        Serial.print("Menu nou: ");
        Serial.println(menu);

        Serial.println("========================");


        reescriure = true;
    }


    // ====================================================
    // BUTTON ENCODER 1
    // ====================================================

    if (
        lastButtonState6 == HIGH &&
        buttonState6 == LOW
    ) {

        Serial.println(
            "ENCODER 1 BUTTON PREMUT"
        );


        ledStrips[0].preset += 1;


        if (
            ledStrips[0].preset
            > NUM_PRESETS
        ) {

            ledStrips[0].preset = 1;
        }


        reescriure = true;
    }


    // ====================================================
    // BUTTON ENCODER 2
    // ====================================================

    if (
        lastButtonState7 == HIGH &&
        buttonState7 == LOW
    ) {

        Serial.println(
            "ENCODER 2 BUTTON PREMUT"
        );


        ledStrips[1].preset += 1;


        if (
            ledStrips[1].preset
            > NUM_PRESETS
        ) {

            ledStrips[1].preset = 1;
        }


        reescriure = true;
    }


    // ====================================================
    // BUTTON ENCODER 3
    // ====================================================

    if (
        lastButtonState8 == HIGH &&
        buttonState8 == LOW
    ) {

        Serial.println(
            "ENCODER 3 BUTTON PREMUT"
        );

        reescriure = true;
    }


    // ====================================================
    // BUTTON ENCODER 4
    // ====================================================

    if (
        lastButtonState9 == HIGH &&
        buttonState9 == LOW
    ) {

        Serial.println(
            "ENCODER 4 BUTTON PREMUT"
        );

        reescriure = true;
    }


    // ====================================================
    // BUTTON ENCODER 5
    // ====================================================

    // ENC5_BTN = -1, per tant no fem digitalRead()
    // ni controlem aquest boto.


    // ====================================================
    // GUARDAR ESTAT DELS BOTONS
    // ====================================================

    lastButtonState1 =
        buttonState1;

    lastButtonState2 =
        buttonState2;

    lastButtonState3 =
        buttonState3;

    lastButtonState4 =
        buttonState4;

    lastButtonState5 =
        buttonState5;

    lastButtonState6 =
        buttonState6;

    lastButtonState7 =
        buttonState7;

    lastButtonState8 =
        buttonState8;

    lastButtonState9 =
        buttonState9;

    lastButtonState10 =
        buttonState10;


    // ====================================================
    // OLED
    // ====================================================

    updateOLED(buf);


    // ====================================================
    // ACTUALITZAR PANTALLES
    // ====================================================

    if (reescriure) {

        updateLCD2004(
            menu,
            menuIndex
        );

        updateLCD1602(
            menu,
            menuIndex
        );

        reescriure = false;
    }

    updateLCDBacklight();
}