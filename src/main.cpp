#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <list>
#include <map>
#include "ArduinoNvs.h"
#include "logging.h"
#include "submitters/submitter.h"
#include "sensors/home_sensor.h"
#include "states/states.h"
#include "config.h"
#include "time/every.h"
#include "time/after.h"

#ifndef VERSION_NUMBER
#define VERSION_NUMBER "0.0.1"
#endif

// General configuration
#define MICROSECONDS_IN_SECOND 1000000L
#define FAILURE_BACKOFF 5L * MICROSECONDS_IN_SECOND
#define REPORT_PERIOD 300L * MICROSECONDS_IN_SECOND

std::list<ReadingSubmitter*> submitters;
std::list<HomeSensor*> sensors;

State currentState = ST_CONFIG_CHECK;

void goToSleep(long microSeconds) {
    std::list<HomeSensor*>::iterator it;
    bool allDone = true;
    for (it = sensors.begin(); it != sensors.end(); ++it)
    {
        HomeSensor *item = (*it);
        item->switchOff();
    }

    delay(20);
    esp_sleep_enable_timer_wakeup(microSeconds);
    esp_deep_sleep_start();
}

static After rebootWait;
static int lastButtonState;

void setup(void)
{
    Serial.begin(115200);
    Serial.printf("Version %s\n", VERSION_NUMBER);
    rebootWait.start();
    lastButtonState = digitalRead(0);
}

static Every eachSecond(1000);
static Every eachHalfSecond(500);

static int pressed = 0;
int released = 0;

void loop(void)
{
    int buttonState = digitalRead(0);
    if (buttonState == 0 && lastButtonState == 1)
    {
        logln("Pressed");
        ++pressed;
    }
    else if (buttonState == 1 && lastButtonState == 0)
    {
        logln("Released");
        ++released;
    }
    lastButtonState = buttonState;

    if (pressed >= 2 && released >= 2)
    {
        logln("Double-click detected - entering re-config mode");
        Config::instance()->reconfigure();
        ESP.restart();
    }

    switch (currentState)
    {
        case ST_CONFIG_CHECK:
            currentState = state_config_check(currentState);
            break;
        case ST_CONFIG_ACTIVATE:
            currentState = state_config_activate(currentState);
            break;
        case ST_CONFIG_ACCEPT:
            currentState = state_config_accept(currentState);
            break;

        case ST_FEATURE_ENABLE:
            currentState = state_feature_enable(currentState, &submitters, &sensors);
            break;

        case ST_NETWORK_ACTIVATE:
            currentState = state_network_activate(currentState);
            break;
        case ST_NETWORK_WAIT:
            currentState = state_network_wait(currentState);
            break;

        case ST_FIRMWARE_CHECK:
            currentState = state_firmware_check(currentState);
            break;

        case ST_SUBMITTER_ACTIVATE:
            currentState = state_submitter_activate(currentState, &submitters);
            break;
        case ST_SUBMITTER_WAIT:
            currentState = state_submitter_wait(currentState, &submitters);
            break;

        case ST_SENSOR_ACTIVATE:
            currentState = state_sensor_activate(currentState, &sensors);
            break;
        case ST_SENSOR_WAIT:
            currentState = state_sensor_wait(currentState, &sensors);
            break;
        case ST_SENSOR_SEND:
            currentState = state_sensor_submit(currentState, &submitters, &sensors);
            break;

        case ST_DONE:
            if (rebootWait.isAfter(3000))
            {
                goToSleep(REPORT_PERIOD);
            }
            else
            {
                eachHalfSecond.run([]{ logln("Delay restart to allow reset button checks"); });
            }
            break;
        case ST_REBOOT:
            ESP.restart();
            break;
        case ST_ERROR:
            goToSleep(FAILURE_BACKOFF);
            break;
        default:
            eachSecond.run([] { logfmt("I don't understand state: %d\n", currentState); });
    }
}