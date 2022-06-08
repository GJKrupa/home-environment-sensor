#pragma once
#include <Wire.h>
#include "sensors/home_sensor.h"

class RainHomeSensor: public HomeSensor {
public:
    RainHomeSensor(int data_pin, int pwr_pin);
    virtual void submitReading(ReadingSubmitter &submitter);
    virtual void setup();
    virtual bool ready();
    virtual bool failed();
    virtual void switchOn();
    virtual void switchOff();
    virtual bool isOn();
    virtual bool sent();
    virtual String name();
private:
    int data_pin;
    int pwr_pin;
    bool data_sent;
    bool is_on;
};