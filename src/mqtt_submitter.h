#pragma once
#include "submitter.h"
#include <Arduino.h>
#include <PubSubClient.h>

class MQTTSubmitter: public ReadingSubmitter {

private:
    String location;
    const char* host;
    int port;
    bool started;
    PubSubClient *client;

public:
    MQTTSubmitter(String location, const char* host, int port);
    virtual bool ready();
    virtual bool failed();
    virtual bool initialised();
    virtual void initialise();
    virtual void sendReading(String name, double value);
};