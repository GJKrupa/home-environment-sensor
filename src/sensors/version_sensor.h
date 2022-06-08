#pragma once
#include <Wire.h>
#include "sensors/home_sensor.h"

class VersionHomeSensor: public HomeSensor {
private:
    bool data_sent;
public:
    VersionHomeSensor();
    virtual void submitReading(ReadingSubmitter &submitter);
    virtual void setup();
    virtual bool ready();
    virtual bool failed();
    virtual void switchOn();
    virtual void switchOff();
    virtual bool isOn();
    virtual bool sent();
    virtual String name();
};