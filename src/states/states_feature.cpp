#include <Arduino.h>
#include <list>
#include "states.h"
#include "submitters/mqtt_submitter.h"
#include "sensors/bme_sensor.h"
#include "sensors/rain_sensor.h"
#include "sensors/battery_sensor.h"
#include "sensors/version_sensor.h"
#include "config.h"
#include "logging.h"

State state_feature_enable(State state, std::list<ReadingSubmitter*>* submitters, std::list<HomeSensor*> *sensors)
{
    Config *config = Config::instance();
    submitters->push_back(new MQTTSubmitter(config->name, config->mqttHost.c_str(), config->mqttPort));
    sensors->push_back(new VersionHomeSensor());
    sensors->push_back(new BMEHomeSensor(25, 26, 32));
    if (config->rainSensor) sensors->push_back(new RainHomeSensor(34, 27));
    if (config->batterySensor) sensors->push_back(new BatteryHomeSensor(33, 32));
    return ST_NETWORK_ACTIVATE;
}