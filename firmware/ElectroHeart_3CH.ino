#include <Arduino.h>
#include "BluetoothSerial.h"

// ElectroHeart 3-channel acquisition firmware
// Target: classic ESP32 / ESP32-WROOM (Bluetooth Classic SPP supported)
// ADC1 inputs: GPIO34, GPIO35, GPIO32
// Binary frame: A5 5A + three uint16 little-endian ADC values
// Sample rate: 2000 frames/second; USB baud: 921600

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error "Bluetooth Classic is not enabled. Select a classic ESP32 board (e.g. ESP32 Dev Module), not ESP32-S3."
#endif

BluetoothSerial SerialBT;
static constexpr uint8_t PIN_MIC1 = 34;
static constexpr uint8_t PIN_MIC2 = 35;
static constexpr uint8_t PIN_MIC3 = 32;
static constexpr uint32_t SAMPLE_RATE = 2000;
static constexpr uint32_t SAMPLE_PERIOD_US = 1000000UL / SAMPLE_RATE;
static uint8_t packet[8] = {0xA5, 0x5A, 0, 0, 0, 0, 0, 0};
static uint32_t nextSample = 0;

static inline void put16(uint8_t *p, uint16_t v) {
  p[0] = (uint8_t)(v & 0xFF);
  p[1] = (uint8_t)(v >> 8);
}

void setup() {
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_MIC1, ADC_11db);
  analogSetPinAttenuation(PIN_MIC2, ADC_11db);
  analogSetPinAttenuation(PIN_MIC3, ADC_11db);
  Serial.begin(921600);
  SerialBT.begin("ESP32-ElectroHeart");
  nextSample = micros();
}

void loop() {
  const uint32_t now = micros();
  if ((int32_t)(now - nextSample) < 0) return;
  nextSample += SAMPLE_PERIOD_US;

  const uint16_t a = (uint16_t)analogRead(PIN_MIC1);
  const uint16_t b = (uint16_t)analogRead(PIN_MIC2);
  const uint16_t c = (uint16_t)analogRead(PIN_MIC3);
  put16(&packet[2], a);
  put16(&packet[4], b);
  put16(&packet[6], c);

  // Stream identical binary frames over USB and Bluetooth SPP.
  Serial.write(packet, sizeof(packet));
  SerialBT.write(packet, sizeof(packet));
}