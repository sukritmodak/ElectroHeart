/*
  ElectroHeart - 3-channel ESP32 USB + Bluetooth SPP waveform streamer
  Board: classic ESP32 / ESP32-WROOM
  Inputs:
    Channel 1: GPIO34 (ADC1)
    Channel 2: GPIO35 (ADC1)
    Channel 3: GPIO32 (ADC1)

  USB Serial AND Bluetooth Classic SPP send the same binary protocol:
    Baud: 921600
    Packet: 8 bytes
      [0] 0xA5
      [1] 0x5A
      [2..3] GPIO34 ADC, uint16 little-endian
      [4..5] GPIO35 ADC, uint16 little-endian
      [6..7] GPIO32 ADC, uint16 little-endian

  IMPORTANT:
  - Upload this sketch to the ESP32.
  - Pair Bluetooth device ESP32-ElectroHeart in Windows first; choose its OUTGOING Bluetooth COM port in Chrome/Edge.
  - Close Arduino Serial Monitor/Plotter before connecting the website.
  - Do not add Serial.print()/Serial.println() while binary streaming.
  - Connect microphone amplifier OUT pins to the listed ADC pins.
  - Power analog modules safely; ESP32 ADC pins must never exceed 3.3V.
*/

#include <Arduino.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

static const uint32_t SERIAL_BAUD = 921600;
static const uint32_t SAMPLE_RATE_HZ = 2000;
static const uint32_t SAMPLE_INTERVAL_US = 1000000UL / SAMPLE_RATE_HZ;

static const int MIC1_PIN = 34;
static const int MIC2_PIN = 35;
static const int MIC3_PIN = 32;

uint32_t nextSampleUs = 0;

static inline void putU16LE(uint8_t *buffer, uint8_t index, uint16_t value) {
  buffer[index] = (uint8_t)(value & 0xFF);
  buffer[index + 1] = (uint8_t)((value >> 8) & 0xFF);
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  SerialBT.begin("ESP32-ElectroHeart");

  analogReadResolution(12);
  analogSetPinAttenuation(MIC1_PIN, ADC_11db);
  analogSetPinAttenuation(MIC2_PIN, ADC_11db);
  analogSetPinAttenuation(MIC3_PIN, ADC_11db);

  pinMode(MIC1_PIN, INPUT);
  pinMode(MIC2_PIN, INPUT);
  pinMode(MIC3_PIN, INPUT);

  nextSampleUs = micros();
}

void loop() {
  const uint32_t now = micros();

  if ((int32_t)(now - nextSampleUs) < 0) {
    return;
  }

  nextSampleUs += SAMPLE_INTERVAL_US;

  // Recover cleanly if the processor was delayed for more than one period.
  if ((int32_t)(now - nextSampleUs) > (int32_t)SAMPLE_INTERVAL_US) {
    nextSampleUs = now + SAMPLE_INTERVAL_US;
  }

  const uint16_t ch1 = (uint16_t)analogRead(MIC1_PIN);
  const uint16_t ch2 = (uint16_t)analogRead(MIC2_PIN);
  const uint16_t ch3 = (uint16_t)analogRead(MIC3_PIN);

  uint8_t packet[8];
  packet[0] = 0xA5;
  packet[1] = 0x5A;
  putU16LE(packet, 2, ch1);
  putU16LE(packet, 4, ch2);
  putU16LE(packet, 6, ch3);

  // Send identical frames over USB and Bluetooth Classic SPP.
  Serial.write(packet, sizeof(packet));
  if (SerialBT.hasClient()) {
    SerialBT.write(packet, sizeof(packet));
  }
}
