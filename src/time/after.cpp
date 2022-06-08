#include <Arduino.h>
#include "after.h"

After::After() : _startTime(0) {
}

void After::start()
{
    _startTime = millis();
}

bool After::isAfter(unsigned long timeout)
{
    return millis() - _startTime > timeout;
}