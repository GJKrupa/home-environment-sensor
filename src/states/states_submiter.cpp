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

State state_submitter_activate(State state, std::list<ReadingSubmitter*>* submitters)
{
    bool allDone = true;
    std::list<ReadingSubmitter*>::iterator it;
    for (it = submitters->begin(); it != submitters->end(); ++it)
    {
        if (!(*it)->initialised())
        {
            allDone = false;
            logfmt("Initialising submitter %s\n", (*it)->name());
            (*it)->initialise();
        }
    }

    timeout.start();

    return allDone ? ST_SUBMITTER_WAIT : ST_SUBMITTER_ACTIVATE;
}

State state_submitter_wait(State state, std::list<ReadingSubmitter*>* submitters)
{
    bool allDone = true;
    std::list<ReadingSubmitter*>::iterator it;
    for (it = submitters->begin(); it != submitters->end(); ++it)
    {
        if ((*it)->failed())
        {
            logfmt("Submitter has failed: %s\n", (*it)->name());
            return ST_ERROR;
        }
        else if (!(*it)->ready() && !(*it)->failed())
        {
            allDone = false;
        }
    }

    if (allDone)
    {
        return ST_SENSOR_ACTIVATE;
    }
    else
    {
        if (timeout.isAfter(20000))
        {
            logln("Reached loop limit, trying again later");
            return ST_ERROR;
        }
        else
        {
            eachSecond.run([]{ logln("Waiting for submitters to be ready"); });
            return ST_SUBMITTER_WAIT;
        }
    }
}