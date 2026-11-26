#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// Minimal advertising diagnostic for classic ESP32 / ESP32-WROOM.
// No microphones, ADC sampling, or data notifications are used.
// Expected visible name: ElectroHeart-TEST

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error "BLE is unavailable for this board configuration. Select a classic ESP32 board, not ESP32-S3."
#endif

static const char* TEST_NAME = "ElectroHeart-TEST";
static const char* TEST_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
static const char* TEST_CHAR_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== ElectroHeart BLE advertising test ===");
  Serial.println("Board should advertise as: ElectroHeart-TEST");

  BLEDevice::init(TEST_NAME);
  BLEServer* server = BLEDevice::createServer();
  BLEService* service = server->createService(TEST_SERVICE_UUID);
  BLECharacteristic* characteristic = service->createCharacteristic(
    TEST_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  characteristic->setValue("BLE-OK");
  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  BLEAdvertisementData primaryAdvertisement;
  primaryAdvertisement.setFlags(0x06);
  primaryAdvertisement.setName(TEST_NAME);
  BLEAdvertisementData scanResponse;
  scanResponse.setCompleteServices(BLEUUID(TEST_SERVICE_UUID));
  advertising->setAdvertisementData(primaryAdvertisement);
  advertising->setScanResponseData(scanResponse);
  advertising->setScanResponse(true);
  advertising->start();

  Serial.println("PASS: BLE stack initialized and advertising start() called.");
  Serial.println("Now scan using Chrome dashboard or a BLE scanner app.");
  Serial.println("If ElectroHeart-TEST is not visible, check board type and the computer BLE adapter.");
}

void loop() {
  delay(1000);
}
