#include <esp32fota.h>
#include "states/states.h"
#include "config.h"
#include "logging.h"

#ifndef VERSION_NUMBER
#define VERSION_NUMBER "0.0.1"
#endif

State state_firmware_check(State state)
{
    if (!Config::instance()->updateUrl.isEmpty())
    {
        esp32FOTA ota("home-sensor", VERSION_NUMBER, false, true);
        ota.checkURL = Config::instance()->updateUrl;
        if (ota.execHTTPcheck())
        {
            logln("Detected a firmare update. Will update now and reboot\n");
            ota.execOTA();
        }
    }
    return ST_SUBMITTER_ACTIVATE;
}