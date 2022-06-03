#pragma once
#include "submitter.h"
#include <Arduino.h>
#include <AsyncMqttClient.h>

class MQTTSubmitter: public ReadingSubmitter {

private:
    String location;
    const char* host;
    int port;
    bool started;
    volatile bool disconnected;
    volatile bool publishAck;
    AsyncMqttClient *client;

    void onDisconnect(AsyncMqttClientDisconnectReason reason);
    void onConnect(bool sessionPresent);
    void onPublishAck(uint16_t packetId);


public:
    MQTTSubmitter(String location, const char* host, int port);
    virtual bool ready();
    virtual bool failed();
    virtual bool initialised();
    virtual void initialise();
    virtual void sendReading(String name, double value);
};