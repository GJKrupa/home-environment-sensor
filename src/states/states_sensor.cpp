#include <Arduino.h>
#include <list>
#include <WiFi.h>
#include "states.h"
#include "config.h"
#include "logging.h"
#include "time/after.h"
#include "time/every.h"

static Every eachSecond(1000);
static After timeout;

State state_sensor_activate(State state, std::list<HomeSensor*>* sensors)
{
    bool allDone = true;
    std::list<HomeSensor*>::iterator it;
    for (it = sensors->begin(); it != sensors->end(); ++it)
    {
        if (!(*it)->isOn())
        {
            allDone = false;
            logfmt("Initialising sensor %s\n", (*it)->name());
            (*it)->switchOn();
            (*it)->setup();
        }
    }

    timeout.start();

    return allDone ? ST_SENSOR_WAIT : ST_SENSOR_ACTIVATE;
}

State state_sensor_wait(State state, std::list<HomeSensor*>* sensors)
{
    bool allDone = true;
    std::list<HomeSensor*>::iterator it;
    for (it = sensors->begin(); it != sensors->end(); ++it)
    {
        if ((*it)->failed())
        {
            logfmt("Sensor has failed: %s\n", (*it)->name());
            return ST_ERROR;
        }
        else if (!(*it)->ready() && !(*it)->failed())
        {
            allDone = false;
        }
    }

    if (allDone)
    {
        return ST_SENSOR_SEND;
    }
    else
    {
        if (timeout.isAfter(20000))
        {
            logln("Reached sensor ready loop limit, trying again later");
            return ST_ERROR;
        }
        else
        {
            eachSecond.run([]{ logln("Waiting for sensors to be ready"); });
            return ST_SENSOR_WAIT;
        }
    }
}

State state_sensor_submit(State state, std::list<ReadingSubmitter*>* submitters, std::list<HomeSensor*> *sensors)
{
    bool allDone = true;
    std::list<HomeSensor*>::iterator it;
    for (it = sensors->begin(); it != sensors->end(); ++it)
    {
        if (!(*it)->sent())
        {
            allDone = false;
            std::list<ReadingSubmitter*>::iterator it2;
            for (it2 = submitters->begin(); it2 != submitters->end(); ++it2)
            {
                timeout.start();
                (*it)->submitReading(**it2);
            }
        }
    }

    std::list<ReadingSubmitter*>::iterator it2;
    for (it2 = submitters->begin(); it2 != submitters->end(); ++it2)
    {
        if (!(*it2)->complete())
        {
            allDone = false;
        }
    }

    if (allDone)
    {
        logln("All reading submitted, shutting down");
        return ST_DONE;
    }
    else
    {
        if (timeout.isAfter(2000))
        {
            logln("Reached sensor submission loop limit, trying again later");
            return ST_ERROR;
        } else {
            return ST_SENSOR_SEND;
        }
    }
}