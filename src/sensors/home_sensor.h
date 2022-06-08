#pragma once
#include "submitters/submitter.h"

class HomeSensor {
public:
    virtual void submitReading(ReadingSubmitter &submitter) = 0;
    virtual void setup() = 0;
    virtual bool sent() = 0;
    virtual bool ready() = 0;
    virtual bool failed() = 0;
    virtual void switchOn() = 0;
    virtual void switchOff() = 0;
    virtual bool isOn() = 0;
    virtual String name() = 0;
};