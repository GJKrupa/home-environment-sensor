#include "sensors/bme_sensor.h"
#include "logging.h"

#define PASCALS_IN_HPA 100.0f

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

uint8_t BMEHomeSensor::getAddress()
{
    uint8_t found = 0;
    byte error, address;
    for (address = 1; address < 127 && found == 0; ++address) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0) {
            found = address;
        }
        else if (error==4)
        {
            logfmt("Err I2C addr %x", address);
        }
    }
    return found;
}

void BMEHomeSensor::setup()
{
    Wire.begin(data_pin, clk_pin);
    uint8_t address = getAddress();
    if (address == 0)
    {
        logln("No i2C on the wire");
        status = false;
    }
    else
    {
        status = bme.begin(address);
        if (!status)
        {
            logln("No BME280 detected");
        }
    }
}

void BMEHomeSensor::submitReading(ReadingSubmitter &submitter)
{
    sensors_event_t temp_event, pressure_event, humidity_event;
    bme.getTemperatureSensor()->getEvent(&temp_event);
    bme.getPressureSensor()->getEvent(&pressure_event);
    bme.getHumiditySensor()->getEvent(&humidity_event);

    submitter.sendReading("temperature", temp_event.temperature);
    submitter.sendReading("humidity", humidity_event.relative_humidity);
    submitter.sendReading("pressure", pressure_event.pressure);
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
        digitalWrite(pwr_pin, HIGH);
    }
    is_on = true;
}

void BMEHomeSensor::switchOff()
{
    if (pwr_pin > 0)
    {
        digitalWrite(pwr_pin, LOW);
    }
    is_on = false;
}