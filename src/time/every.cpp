#include <Arduino.h>
#include "every.h"

Every::Every(unsigned long period) : 
    _lastTime(0),
    _period(period) 
{
}

void Every::run(std::function<void(void)> fn)
{
    if (_lastTime == 0 || millis() - _lastTime >= _period)
    {
        fn();
        _lastTime = millis();
    }
}
