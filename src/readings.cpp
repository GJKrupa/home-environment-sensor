#include<readings.h>
#include <Arduino.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

Readings::Readings(const char* const name):
    hasConnected(false),
    hasDisconnected(false)
{
    BLEDevice::init(name);
    server = BLEDevice::createServer();
    server->setCallbacks(this);
    service = server->createService(SERVICE_UUID);
}

void Readings::setValue(const char* const uuid, double value) {
    Serial.print("Adding ");
    Serial.println(uuid);
    BLECharacteristic *characteristic = service->createCharacteristic(uuid, BLECharacteristic::PROPERTY_READ);
    characteristic->addDescriptor(new BLE2902());
    characteristic->setValue(value);
}

void Readings::start() {
    service->start();
    advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMaxPreferred(0x06);
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Started Adverising");
}

void Readings::onConnect(BLEServer* pServer) {
    Serial.println("Connected");
    hasConnected = true;
};

void Readings::onDisconnect(BLEServer* pServer) {
    Serial.println("Disonnected");
    if (hasConnected) {
        hasDisconnected = true;
    }
}

bool Readings::readCompleted() {
    return hasConnected && hasDisconnected;
}