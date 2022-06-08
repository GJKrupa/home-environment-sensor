#include <Arduino.h>
#include <list>
#include <WiFi.h>
#include "states.h"
#include "config.h"
#include "logging.h"
#include "time/after.h"
#include "time/every.h"

static Every eachSecond(1000);
static After networkTimeout;

State state_network_activate(State state)
{
    Config *config = Config::instance();
    WiFi.begin(config->ssid.c_str(), config->passphrase.c_str());
    networkTimeout.start();
    return ST_NETWORK_WAIT;
}

State state_network_wait(State state)
{
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED)
    {
        log_init(Config::instance()->name);
        return ST_FIRMWARE_CHECK;
    }
    else if (status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST)
    {
        logln("Wifi has failed");
        return ST_ERROR;
    }
    else if (networkTimeout.isAfter(20000))
    {
        logln("Reached loop limit, trying again later");
        return ST_ERROR;
    }
    else
    {
        eachSecond.run([]{ logln("Waiting for WiFi"); });
        return state;
    }
    
}