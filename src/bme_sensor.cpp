#include "bme_sensor.h"
#include "logging.h"

#define PASCALS_IN_HPA 100.0f
#ifndef ALTITUDE
#define ALTITUDE 80.0f
#endif

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
            logf("Unknow error scanning I2C at address 0x%x", address);
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
        logln("Unable to find an I2C device on the selected wire");
        status = false;
    }
    else
    {
        logf("Found an I2C device ID of 0x%x\n", bme.sensorID());
        status = bme.begin(address);
        if (!status)
        {
            logln("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
            logln("ID of:        0xFF probably means a bad address, a BMP 180 or BMP 085");
            logln("         0x56-0x58 represents a BMP 280,");
            logln("              0x60 represents a BME 280,");
            logln("              0x61 represents a BME 680.");
        }
    }
}

void BMEHomeSensor::submitReading(ReadingSubmitter &submitter)
{
    logln("Getting BME sensors");

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