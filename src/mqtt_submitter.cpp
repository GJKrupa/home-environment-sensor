#include <functional>
#include "mqtt_submitter.h"
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "logging.h"

MQTTSubmitter::MQTTSubmitter(String location, const char* host, int port) :
    location(location),
    host(host),
    port(port),
    started(false),
    disconnected(false),
    publishAck(false),
    client(new AsyncMqttClient())
{
}

bool MQTTSubmitter::initialised()
{
    return started;
}

void MQTTSubmitter::initialise()
{
    client->setServer(host, port);
    client->setClientId("ESP32");
    client->onConnect(
        [this] (bool sessionPresent) { this->onConnect(sessionPresent); }
    );
    client->onDisconnect(
        [this] (AsyncMqttClientDisconnectReason reason) { this->onDisconnect(reason); }
    );
    client->onPublish(
        [this] (uint16_t packetId) { this->onPublishAck(packetId); }
    );
    client->connect();
    started = true;
}

bool MQTTSubmitter::ready()
{
    return client->connected();
}

bool MQTTSubmitter::failed()
{
    return disconnected;
}

void MQTTSubmitter::onConnect(bool sessionPresent)
{
  logln("Connected to MQTT.");
  logf("Session present: %s\n", sessionPresent ? "true" : "false");
}

void MQTTSubmitter::onDisconnect(AsyncMqttClientDisconnectReason reason)
{
    logln("Disconnected from MQTT.");
    disconnected = true;
}

void MQTTSubmitter::onPublishAck(uint16_t packetId)
{
  logf("Publish acknowledged, packetId: %d\n", packetId);
  publishAck = true;
}

void MQTTSubmitter::sendReading(String name, double value)
{
    String topic = "environment/";
    topic.concat(location);
    topic.concat("/");
    topic.concat(name);

    String valueString = String(value);

    logf("Submitting reading %s = %f\n", topic.c_str(), value);

    publishAck = false;
    uint16_t msgId = client->publish(topic.c_str(), 1, true, valueString.c_str(), valueString.length());
    for (int count = 0; count < 100 && !publishAck; ++count)
    {
        delay(10);
    }
    if (publishAck)
    {
        logf("Got ack on message ID: %d\n", msgId);
    }
    else
    {
        logf("Time out waiting for ack on message ID: %d\n", msgId);
    }
    
}