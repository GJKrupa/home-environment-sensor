#include <functional>
#include "mqtt_submitter.h"
#include <WiFi.h>
#include <AsyncMqttClient.h>

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
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void MQTTSubmitter::onDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");
    disconnected = true;
}

void MQTTSubmitter::onPublishAck(uint16_t packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  publishAck = true;
}

void MQTTSubmitter::sendReading(String name, double value)
{
    String topic = "environment/";
    topic.concat(location);
    topic.concat("/");
    topic.concat(name);

    String valueString = String(value);

    Serial.printf("Submitting reading %s = %f\n", topic.c_str(), value);

    publishAck = false;
    uint16_t msgId = client->publish(topic.c_str(), 1, true, valueString.c_str(), valueString.length());
    for (int count = 0; count < 100 && !publishAck; ++count)
    {
        delay(10);
    }
    if (publishAck)
    {
        Serial.printf("Got ack on message ID: %d\n", msgId);
    }
    else
    {
        Serial.printf("Time out waiting for ack on message ID: %d\n", msgId);
    }
    
}