#pragma once
#include "submitter.h"
#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <set>

class MQTTSubmitter: public ReadingSubmitter {

private:
    String location;
    const char* host;
    int port;
    bool started;
    volatile bool disconnected;
    AsyncMqttClient *client;
    std::set<uint16_t> pendingAcks;

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
    virtual void sendReading(String name, String value);
    virtual const char *name();
    virtual bool complete();
};