#pragma once
#include <list>
#include "submitters/submitter.h"
#include "sensors/home_sensor.h"

enum State
{
    ST_DONE,
    ST_REBOOT,
    ST_ERROR,
    ST_CONFIG_CHECK,
    ST_CONFIG_ACTIVATE,
    ST_CONFIG_ACCEPT,
    ST_FEATURE_ENABLE,
    ST_NETWORK_ACTIVATE,
    ST_NETWORK_WAIT,
    ST_FIRMWARE_CHECK,
    ST_SUBMITTER_ACTIVATE,
    ST_SUBMITTER_WAIT,
    ST_SENSOR_ACTIVATE,
    ST_SENSOR_WAIT,
    ST_SENSOR_SEND
};

State state_config_check(State state);
State state_config_activate(State state);
State state_config_accept(State state);

State state_feature_enable(State state, std::list<ReadingSubmitter*>* submitters, std::list<HomeSensor*> *sensors);

State state_network_activate(State state);
State state_network_wait(State state);

State state_firmware_check(State state);

State state_submitter_activate(State state, std::list<ReadingSubmitter*>* submitters);
State state_submitter_wait(State state, std::list<ReadingSubmitter*>* submitters);

State state_sensor_activate(State state, std::list<HomeSensor*>* sensors);
State state_sensor_wait(State state, std::list<HomeSensor*>* sensors);
State state_sensor_submit(State state, std::list<ReadingSubmitter*>* submitters, std::list<HomeSensor*> *sensors);