#include <functional>
#include <set>
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
    pendingAcks(),
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
  logln("MQTT is up");
}

void MQTTSubmitter::onDisconnect(AsyncMqttClientDisconnectReason reason)
{
    logfmt("MQTT is down because %d\n", reason);
    disconnected = true;
}

void MQTTSubmitter::onPublishAck(uint16_t packetId)
{
  pendingAcks.erase(packetId);
}

void MQTTSubmitter::sendReading(String name, double value)
{
    String topic = "environment/";
    topic.concat(location);
    topic.concat("/");
    topic.concat(name);

    String valueString = String(value);

    logfmt("MQTT <- %s = %f\n", topic.c_str(), value);

    uint16_t msgId = client->publish(topic.c_str(), 1, true, valueString.c_str(), valueString.length());
    pendingAcks.insert(msgId);
}

void MQTTSubmitter::sendReading(String name, String value)
{
    String topic = "environment/";
    topic.concat(location);
    topic.concat("/");
    topic.concat(name);

    logfmt("MQTT <- %s = %s\n", topic.c_str(), value.c_str());

    uint16_t msgId = client->publish(topic.c_str(), 1, true, value.c_str(), value.length());
    pendingAcks.insert(msgId);
}

bool MQTTSubmitter::complete()
{
    return pendingAcks.empty();
}

const char *MQTTSubmitter::name()
{
    return "MQTT";
}