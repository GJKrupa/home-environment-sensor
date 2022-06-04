#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include "mqtt_submitter.h"
#include <list>
#include "home_sensor.h"
#include "bme_sensor.h"
#include "rain_sensor.h"
#include "battery_sensor.h"
#include "logging.h"

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
        logf("Switching off: %s\n", item->name());
        item->switchOff();
    }

    log_close();
    delay(20);
    esp_sleep_enable_timer_wakeup(microSeconds);
    esp_deep_sleep_start();
}

void setup(void)
{
    executionStart = esp_timer_get_time();
    wakeupCause = esp_sleep_get_wakeup_cause();

    log_init();

    logf("Connecting to wifi %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);

    sensors.push_back(new BMEHomeSensor(25, 26, 32));
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
        logln("Reached loop limit, trying again later");
        goToSleep(FAILURE_BACKOFF);
    }
    else if (status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST)
    {
        logln("Wifi has failed");
        goToSleep(FAILURE_BACKOFF);
    }
    else if (!WiFi.isConnected())
    {
        logln("Wifi not yet connected");
        delay(1000);
    }
    else if (submitter.failed())
    {
        logln("Submitter has failed");
        goToSleep(FAILURE_BACKOFF);
    }
    else if (!submitter.initialised())
    {
        logln("Initialising submitter");
        submitter.initialise();
        loopCount = 0;
    }
    else if (!submitter.ready())
    {
        logln("Submitter is not ready");
        delay(1000);
    }
    else
    {
        std::list<HomeSensor*>::iterator it;

        if (complete())
        {
            logln("All reporting is complete");
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
                    logf("Item failed: %s\n", item->name());
                    // No-op
                }
                else if (!item->sent() && !item->isOn())
                {
                    logf("Switching on: %s\n", item->name());
                    item->switchOn();
                    item->setup();
                }
                else if (!item->sent() && item->ready())
                {
                    logf("Submitting reading: %s\n", item->name());
                    item->submitReading(submitter);
                }
                else
                {
                    logf("Sensor is not ready: %s\n", item->name());
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