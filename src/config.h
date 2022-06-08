#pragma once
#include<Arduino.h>

#define CONFIG_MODE_UNSET 0
#define CONFIG_MODE_SET 1
#define CONFIG_MODE_RESET 2

class Config
{
    private:
        static Config *_instance;

    public:
        int mode;
        int version;
        String name;
        String ssid;
        String passphrase;
        String mqttHost;
        int mqttPort;
        String syslogHost;
        int syslogPort;
        String updateUrl;
        bool rainSensor;
        bool batterySensor;
        bool syslogEnabled;

        Config();

        void load();
        void save();
        void reconfigure();

        static Config* instance();
};