#include "sensors/version_sensor.h"

#ifndef VERSION_NUMBER
#define VERSION_NUMBER "0.0.1"
#endif

VersionHomeSensor::VersionHomeSensor() :
    data_sent(false)
{
}

String VersionHomeSensor::name()
{
    return "Version";
}

void VersionHomeSensor::setup()
{
}

void VersionHomeSensor::submitReading(ReadingSubmitter &submitter)
{
    submitter.sendReading("version", VERSION_NUMBER);
    data_sent = true;
}

bool VersionHomeSensor::ready()
{
    return true;
}

bool VersionHomeSensor::failed()
{
    return false;
}

bool VersionHomeSensor::sent()
{
    return data_sent;
}

bool VersionHomeSensor::isOn()
{
    return true;
}

void VersionHomeSensor::switchOn()
{
}

void VersionHomeSensor::switchOff()
{
}