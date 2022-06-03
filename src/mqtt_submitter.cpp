#include "mqtt_submitter.h"
#include <WiFi.h>
#include <PubSubClient.h>

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

MQTTSubmitter::MQTTSubmitter(String location, const char* host, int port) :
    location(location),
    host(host),
    port(port),
    started(false),
    client(new PubSubClient(*new WiFiClient()))
{
}

bool MQTTSubmitter::initialised()
{
    return started;
}

void MQTTSubmitter::initialise()
{
    client->setServer(host, port);
    client->setCallback(callback);
    client->connect("ESP32");
    started = true;
}

bool MQTTSubmitter::ready()
{
    return client->connected() || client->connect("ESP32");
}

bool MQTTSubmitter::failed()
{
    return false;
}

void MQTTSubmitter::sendReading(String name, double value)
{
    String topic = "environment/";
    topic.concat(location);
    topic.concat("/");
    topic.concat(name);

    String valueString = String(value);

    Serial.printf("Submitting reading %s = %f\n", topic.c_str(), value);

    client->publish(topic.c_str(), valueString.c_str());
}