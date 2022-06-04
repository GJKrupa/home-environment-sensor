#pragma once
#include <Wire.h>
#include <BME280_t.h>
#include "home_sensor.h"

class BMEHomeSensor: public HomeSensor {
public:
    BMEHomeSensor(int data_pin, int clk_pin, int pwr_pin);
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
    int clk_pin;
    int pwr_pin;
    bool status;
    bool data_sent;
    bool is_on;
    BME280<> bme;
};