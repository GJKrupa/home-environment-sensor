#include "sensors/battery_sensor.h"
#include "logging.h"

#define MAX_VOLTAGE 4.4 * 5.0

BatteryHomeSensor::BatteryHomeSensor(int data_pin, int pwr_pin) :
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

String BatteryHomeSensor::name()
{
    return "Battery";
}

void BatteryHomeSensor::setup()
{
}

void BatteryHomeSensor::submitReading(ReadingSubmitter &submitter)
{
    logln("Getting Battery sensors");

    uint16_t analogueReading = analogRead(data_pin);
    double actual = ((double)analogueReading / 4095.0) * MAX_VOLTAGE;

    submitter.sendReading("battery", actual);
    data_sent = true;
}

bool BatteryHomeSensor::ready()
{
    return true;
}

bool BatteryHomeSensor::failed()
{
    return false;
}

bool BatteryHomeSensor::sent()
{
    return data_sent;
}

bool BatteryHomeSensor::isOn()
{
    return is_on;
}

void BatteryHomeSensor::switchOn()
{
    if (pwr_pin > 0)
    {
        digitalWrite(pwr_pin, HIGH);
    }
    is_on = true;
}

void BatteryHomeSensor::switchOff()
{
    if (pwr_pin > 0)
    {
        digitalWrite(pwr_pin, LOW);
    }
    is_on = false;
}