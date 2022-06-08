#pragma once

class After {
    private:
        unsigned long _startTime;

    public:
        After();
        void start();
        bool isAfter(unsigned long timeout);
};