#include<BLEDevice.h>
#include<BLEUtils.h>
#include<BLEServer.h>
#include<BLE2902.h>

#define TEMPERATURE_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BATTERY_UUID "5b1363f0-9f57-11ea-a61f-f32afc9842aa"
#define RAIN_UUID "7649df80-9f59-11ea-8bb5-a35ae24ada32"
#define HUMIDITY_UUID "7cd6d060-9f59-11ea-8bb5-a35ae24ada32"
#define PRESSURE_UUID "8aabebd0-9f59-11ea-8bb5-a35ae24ada32"

class Readings: public BLEServerCallbacks {
    private:
        BLEServer *server;
        BLEService *service;
        BLEAdvertising *advertising;
        bool hasConnected;
        bool hasDisconnected;
    public:
        Readings(const char* const);
        void setValue(const char*, double);
        void start();
        bool readCompleted();
        void onConnect(BLEServer*);
        void onDisconnect(BLEServer*);
};