#pragma once
#include <Arduino.h>

class ReadingSubmitter {
public:
    virtual bool ready() = 0;
    virtual bool failed() = 0;
    virtual bool initialised() = 0;
    virtual void initialise() = 0;
    virtual void sendReading(String name, double value) = 0;
    virtual void sendReading(String name, String value) = 0;
    virtual const char *name();
    virtual bool complete();
};