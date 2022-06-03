#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include "mqtt_submitter.h"
#include <list>
#include "home_sensor.h"
#include "bme_sensor.h"
#include "rain_sensor.h"
#include "battery_sensor.h"

// General configuration
#define MICROSECONDS_IN_SECOND 1000000L
#define FAILURE_BACKOFF 5L * MICROSECONDS_IN_SECOND
#define REPORT_PERIOD 120L * MICROSECONDS_IN_SECOND

// Will be overridden in config
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSPHRASE
#define WIFI_PASSPHRASE ""
#endif

// Instance-specific configuration

#ifndef ROOM
#define ROOM "nowhere"
#endif

#ifndef RAIN_MONITOR
#define RAIN_MONITOR 0
#endif

RTC_DATA_ATTR double lastExecutionTime;


esp_sleep_wakeup_cause_t wakeupCause;
long executionStart;

MQTTSubmitter submitter(ROOM, MQTT_SERVER, MQTT_PORT);
std::list<HomeSensor*> sensors;

void goToSleep(long microSeconds) {
    std::list<HomeSensor*>::iterator it;
    bool allDone = true;
    for (it = sensors.begin(); it != sensors.end(); ++it)
    {
        HomeSensor *item = (*it);
        Serial.printf("Switching off: %s\n", item->name());
        item->switchOff();
    }

    Serial.flush();
    delay(20);
    esp_sleep_enable_timer_wakeup(microSeconds);
    esp_deep_sleep_start();
}

void setup(void)
{
    executionStart = esp_timer_get_time();
    wakeupCause = esp_sleep_get_wakeup_cause();

    Serial.begin(115200);

    Serial.printf("Connecting to wifi %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);

    sensors.push_back(new BMEHomeSensor(25, 26, -1));
#if RAIN_MONITOR == 1
    sensors.push_back(new RainHomeSensor(34, 27));
#endif
#if BATTERY_MONITOR == 1
    sensors.push_back(new  BatteryHomeSensor(33, 32));
#endif
}

bool complete()
{
    std::list<HomeSensor*>::iterator it;
    bool allDone = true;
    for (it = sensors.begin(); it != sensors.end(); ++it)
    {
        HomeSensor *item = (*it);
        if (!item->isOn() || !(item->sent() || item->failed()))
        {
            allDone = false;
        }
    }
    return allDone;
}

int loopCount = 0;

void loop(void)
{
    wl_status_t status = WiFi.status();

    if (++loopCount > 20)
    {
        Serial.println("Reached loop limit, trying again later");
        goToSleep(FAILURE_BACKOFF);
    }
    else if (status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST)
    {
        Serial.println("Wifi has failed");
        goToSleep(FAILURE_BACKOFF);
    }
    else if (!WiFi.isConnected())
    {
        Serial.println("Wifi not yet connected");
        delay(1000);
    }
    else if (submitter.failed())
    {
        Serial.println("Submitter has failed");
        goToSleep(FAILURE_BACKOFF);
    }
    else if (!submitter.initialised())
    {
        Serial.println("Initialising submitter");
        submitter.initialise();
        loopCount = 0;
    }
    else if (!submitter.ready())
    {
        Serial.println("Submitter is not ready");
        delay(1000);
    }
    else
    {
        std::list<HomeSensor*>::iterator it;

        if (complete())
        {
            Serial.println("All reporting is complete");
            goToSleep(REPORT_PERIOD);
        }
        else
        {
            bool mustWait = false;
            for (it = sensors.begin(); it != sensors.end(); ++it)
            {
                HomeSensor *item = (*it);
                if (item->isOn() && item->failed())
                {
                    Serial.printf("Item failed: %s\n", item->name());
                    // No-op
                }
                else if (!item->sent() && !item->isOn())
                {
                    Serial.printf("Switching on: %s\n", item->name());
                    item->switchOn();
                    item->setup();
                }
                else if (!item->sent() && item->ready())
                {
                    Serial.printf("Submitting reading: %s\n", item->name());
                    item->submitReading(submitter);
                }
                else
                {
                    Serial.printf("Sensor is not ready: %s\n", item->name());
                    mustWait = true;
                }
            }
            if (mustWait)
            {
                delay(200);
            }
        }
    }
}