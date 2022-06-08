#pragma once

#include<functional>

class Every {
    private:
        unsigned long _lastTime;
        unsigned long _period;

    public:
        Every(unsigned long period);
        void run(std::function<void(void)> fn);
};