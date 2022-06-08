#include "sensors/rain_sensor.h"

#define PASCALS_IN_HPA 100.0f

RainHomeSensor::RainHomeSensor(int data_pin, int pwr_pin) :
    data_pin(data_pin),
    pwr_pin(pwr_pin),
    data_sent(false),
    is_on(false)
{
    if (pwr_pin > 0)
    {
        pinMode(pwr_pin, OUTPUT);
    }
    pinMode(data_pin, INPUT);
}

String RainHomeSensor::name()
{
    return "Rain";
}

void RainHomeSensor::setup()
{
}

void RainHomeSensor::submitReading(ReadingSubmitter &submitter)
{
    uint16_t analogueReading = analogRead(data_pin);
    int invertedReading = 4095 - analogueReading;
    double actual = ((double)invertedReading / 4095.8) * 100.0;
    submitter.sendReading("rain", actual);
    data_sent = true;
}

bool RainHomeSensor::ready()
{
    return true;
}

bool RainHomeSensor::failed()
{
    return false;
}

bool RainHomeSensor::sent()
{
    return data_sent;
}

bool RainHomeSensor::isOn()
{
    return is_on;
}

void RainHomeSensor::switchOn()
{
    if (pwr_pin > 0)
    {
        digitalWrite(pwr_pin, HIGH);
    }
    is_on = true;
}

void RainHomeSensor::switchOff()
{
    if (pwr_pin > 0)
    {
        digitalWrite(pwr_pin, LOW);
    }
    is_on = false;
}