#include "bme_sensor.h"
#include "logging.h"

#define PASCALS_IN_HPA 100.0f
#define ALTITUDE 80.0f

BMEHomeSensor::BMEHomeSensor(int data_pin, int clk_pin, int pwr_pin) :
    data_pin(data_pin),
    clk_pin(clk_pin),
    pwr_pin(pwr_pin),
    status(false),
    data_sent(false),
    is_on(false),
    bme()
{
    if (pwr_pin > 0)
    {
        pinMode(pwr_pin, OUTPUT);
    }
}

String BMEHomeSensor::name()
{
    return "BME280";
}

void BMEHomeSensor::setup()
{
    Wire.begin(data_pin, clk_pin);
    status = bme.begin();
}

void BMEHomeSensor::submitReading(ReadingSubmitter &submitter)
{
    logln("Getting BME sensors");
    bme.refresh();
    submitter.sendReading("temperature", bme.temperature);
    submitter.sendReading("humidity", bme.humidity);
    submitter.sendReading("pressure", bme.seaLevelForAltitude(ALTITUDE) / PASCALS_IN_HPA);
    data_sent = true;
}

bool BMEHomeSensor::ready()
{
    return status;
}

bool BMEHomeSensor::failed()
{
    return !status;
}

bool BMEHomeSensor::sent()
{
    return data_sent;
}

bool BMEHomeSensor::isOn()
{
    return is_on;
}

void BMEHomeSensor::switchOn()
{
    if (pwr_pin > 0)
    {
        digitalWrite(32, HIGH);
    }
    is_on = true;
}

void BMEHomeSensor::switchOff()
{
    if (pwr_pin > 0)
    {
        digitalWrite(32, LOW);
    }
    is_on = false;
}