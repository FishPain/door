#include <Arduino.h>
#include "motor.h"

// === Configuration ===
#define MOTOR_PWM_PIN 10     // PWM-capable pin to DRV8833 IN1
#define ENABLE_PIN    4      // Optional: for DRV8833 sleep/enable control

#define PWM_CHANNEL   0
#define PWM_FREQ      1000   // 1 kHz PWM frequency
#define PWM_RES       8      // 8-bit resolution (0–255)

// === Setup motor PWM and enable ===
void setupMotor() {
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH); // Enable driver (if needed)

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(MOTOR_PWM_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0); // start with motor off
}

// === Drive motor with given speed (0–255) ===
void driveMotor(uint8_t speed) {
  speed = constrain(speed, 0, 255);
  ledcWrite(PWM_CHANNEL, speed);
}

// === Optional: Stop motor ===
void stopMotor() {
  ledcWrite(PWM_CHANNEL, 0);
}