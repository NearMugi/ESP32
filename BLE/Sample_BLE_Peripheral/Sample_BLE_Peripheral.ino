//Nefry BLE(Peripheral)
//スケッチサイズが大きくてコンパイルが通らない。
//https://dotstud.io/blog/nefrybt-ble-bluetooth-peripheral/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define DEVICE "NefryBT"
#define SERVICE_UUID        "D5875408-FA51-4763-A75D-7D33CECEBC31"
#define CHARACTERISTIC_UUID "A4F01D8C-A037-43B6-9050-1876A8C23584"
#define BLE_PROPERTY BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(DEVICE);

#if false
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLE_PROPERTY);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println(F("Waiting a client connection to notify..."));
#endif
}

void loop() {
#if false
  if (deviceConnected) {
    Serial.printf("*** NOTIFY: %d ***\n", value);
    char buffer[10];
    sprintf(buffer, "{\"val\":%d}", value);
    Serial.printf(buffer);
    pCharacteristic->setValue(buffer);
    pCharacteristic->notify();
    //pCharacteristic->indicate();
    value++;
  }
  delay(2000);
#endif
}
