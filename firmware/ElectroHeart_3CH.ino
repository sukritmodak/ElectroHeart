#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ElectroHeart BLE-only firmware for classic ESP32 / ESP32-WROOM.
// Three KY-037/KY-038 analog outputs (AO): GPIO 34, 35, 32. Sample rate: 2000 Hz.
// Power microphone modules from 3.3 V, share GND, and never exceed 3.3 V on AO.
// Each 8-byte frame: A5 5A + three uint16 little-endian ADC values.
// Two frames are batched into each 16-byte BLE notification (within the default ATT MTU).

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error "BLE unavailable: select a classic ESP32 board such as ESP32 Dev Module, not ESP32-S3."
#endif

static constexpr uint8_t PIN_MIC1=34, PIN_MIC2=35, PIN_MIC3=32;
static constexpr uint32_t SAMPLE_RATE=2000, SAMPLE_PERIOD_US=1000000UL/SAMPLE_RATE;
static constexpr size_t FRAME_SIZE=8, FRAMES_PER_PACKET=2, PACKET_SIZE=FRAME_SIZE*FRAMES_PER_PACKET;
static const char* DEVICE_NAME="ElectroHeart-BLE";
static const char* SERVICE_UUID="6e400001-b5a3-f393-e0a9-e50e24dcca9e";
static const char* TX_UUID="6e400003-b5a3-f393-e0a9-e50e24dcca9e";

BLECharacteristic* txCharacteristic=nullptr;
uint8_t notifyBuffer[PACKET_SIZE];
size_t notifyLength=0;
uint32_t nextSample=0;
bool clientConnected=false;

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer*) override { clientConnected=true; notifyLength=0; Serial.println("BLE client connected"); }
  void onDisconnect(BLEServer*) override { clientConnected=false; notifyLength=0; Serial.println("BLE client disconnected; restarting advertising"); BLEDevice::startAdvertising(); }
};

static inline void put16(uint8_t* p,uint16_t v){p[0]=(uint8_t)(v&0xFF);p[1]=(uint8_t)(v>>8);}
static void appendSample(){
  uint8_t* p=&notifyBuffer[notifyLength];
  p[0]=0xA5;p[1]=0x5A;
  put16(p+2,(uint16_t)analogRead(PIN_MIC1));
  put16(p+4,(uint16_t)analogRead(PIN_MIC2));
  put16(p+6,(uint16_t)analogRead(PIN_MIC3));
  notifyLength+=FRAME_SIZE;
}

void setup(){
  Serial.begin(115200); // USB is for diagnostics/firmware upload only; PCG data remains BLE-only.
  delay(300);
  Serial.println();
  Serial.println("ElectroHeart BLE booting...");
  Serial.println("Expected BLE name: ElectroHeart-BLE");
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_MIC1,ADC_11db);
  analogSetPinAttenuation(PIN_MIC2,ADC_11db);
  analogSetPinAttenuation(PIN_MIC3,ADC_11db);
  BLEDevice::init(DEVICE_NAME);
  BLEDevice::setMTU(185);
  Serial.println("BLE stack initialized");
  BLEServer* server=BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  BLEService* service=server->createService(SERVICE_UUID);
  txCharacteristic=service->createCharacteristic(TX_UUID,BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_NOTIFY);
  txCharacteristic->addDescriptor(new BLE2902());
  service->start();
  BLEAdvertising* advertising=BLEDevice::getAdvertising();
  // Advertise the local name directly in the primary packet.
  // The dashboard uses acceptAllDevices and discovers the GATT service after selection.
  // This avoids depending on scan-response support in the host scanner.
  advertising->setScanResponse(false);
  advertising->start();
  Serial.println("BLE advertising start requested (primary local name enabled).");
  Serial.println("Open Chrome dashboard and choose ElectroHeart-BLE");
  nextSample=micros();
}

void loop(){
  uint32_t now=micros();
  if((int32_t)(now-nextSample)<0)return;
  nextSample+=SAMPLE_PERIOD_US;
  appendSample();
  if(notifyLength>=PACKET_SIZE){
    if(clientConnected&&txCharacteristic){
      txCharacteristic->setValue(notifyBuffer,notifyLength);
      txCharacteristic->notify();
    }
    notifyLength=0;
  }
}
