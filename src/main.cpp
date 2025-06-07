#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

#include "accesspoint.h"
#include "messager.h"

#define SETUP_BUTTON 9
#define HOLD_TIME 3000  // 3 seconds

enum SystemState {
  STATE_READY,
  STATE_SETUP
};

SystemState currentState = STATE_READY;

unsigned long buttonPressTime = 0;
bool holdTriggered = false;

void enterSetupMode() {
  Serial.println("🔧 Entering SETUP mode...");
  currentState = STATE_SETUP;
  startAPMode();
  Serial.println("🌐 Setup mode started. Connect to the ESP32 AP to configure Wi-Fi.");
}

void enterReadyMode() {
  Serial.println("✅ Entering READY mode...");
  currentState = STATE_READY;
  
  if (!connectToSavedWiFi()) {
    Serial.println("⚠️ No Wi-Fi. Hold button to re-enter setup.");
  } else {
    Serial.println("📶 Wi-Fi connected.");
    
    setupMessanger();  // Initialize WebSocket connection
  }
}

// 🟡 Decoupled button logic
void checkButtonHoldToggle() {
  int buttonState = digitalRead(SETUP_BUTTON);
  if (buttonState == LOW) {
    if (buttonPressTime == 0) {
      buttonPressTime = millis();
    }

    if (!holdTriggered && millis() - buttonPressTime >= HOLD_TIME) {
      // Toggle state
      if (currentState == STATE_READY) {
        enterSetupMode();
      } else {
        enterReadyMode();
      }
      holdTriggered = true;
    }
  } else {
    buttonPressTime = 0;
    holdTriggered = false;
  }
}

void setup() {
  Serial.begin(115200);
  
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  
  pinMode(SETUP_BUTTON, INPUT_PULLUP);
  
  Serial.println("🚀 ESP32-C3 Booting...");
  
  // clearPreferences();  // Clear any previous settings
  if (!connectToSavedWiFi()) {
    enterSetupMode();  // fallback to AP mode
  } else {
    enterReadyMode();
  }
}
void loop() {
  checkButtonHoldToggle();

  switch (currentState) {
    case STATE_READY:
      handleWebServer();   // optional, only if you're using a web server
      handleMessanger();   // required for MQTT to keep working
      break;

    case STATE_SETUP:
      handleAPServer();
      break;
  }

  delay(50);  // Consider reducing for snappier response
}