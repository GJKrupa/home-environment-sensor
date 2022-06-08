#include "ArduinoNvs.h"
#include "config.h"
#include "logging.h"

Config *Config::_instance = nullptr;

Config::Config() :
    mode(0),
    version(0),
    name(""),
    ssid(""),
    passphrase(""),
    mqttHost(""),
    mqttPort(1883),
    syslogHost(""),
    syslogPort(514),
    updateUrl(""),
    rainSensor(false),
    batterySensor(false),
    syslogEnabled(false)
{
    NVS.begin();
}

void Config::load()
{
    mode = NVS.getInt("X");
    if (mode != CONFIG_MODE_UNSET)
    {
        version = NVS.getInt("v");
        name = NVS.getString("n");
        ssid = NVS.getString("ss");
        passphrase = NVS.getString("pp");
        mqttHost = NVS.getString("mh");
        mqttPort = NVS.getInt("mp");
        syslogHost = NVS.getString("sh");
        syslogPort = NVS.getInt("sp");
        updateUrl = NVS.getString("uu");
        rainSensor = NVS.getInt("rs") != 0;
        batterySensor = NVS.getInt("bs") != 0;

        logfmt("Version: %d\n", version);
        logfmt("name: %s\n", name.c_str());
        logfmt("ssid: %s\n", ssid.c_str());
        logfmt("passphrase: %s\n", passphrase.c_str());
        logfmt("mqttHost: %s\n", mqttHost.c_str());
        logfmt("mqttPort: %d\n", mqttPort);
        logfmt("syslogHost: %s\n", syslogHost.c_str());
        logfmt("syslogPort: %d\n", syslogPort);
        logfmt("updateUrl: %s\n", updateUrl.c_str());
        logfmt("rainSensor: %s\n", rainSensor ? "true" : "false");
        logfmt("batterySensor: %s\n", batterySensor ? "true" : "false");

        if (!syslogHost.isEmpty())
        {
            syslogEnabled = true;
        }
    }
}

void Config::save()
{
    NVS.setInt("X", CONFIG_MODE_SET);
    NVS.setInt("v", version);
    NVS.setString("n", name);
    NVS.setString("ss", ssid);
    NVS.setString("pp", passphrase);
    NVS.setString("mh", mqttHost);
    NVS.setInt("mp", mqttPort);
    NVS.setString("sh", syslogHost);
    NVS.setInt("sp", syslogPort);
    NVS.setString("uu", updateUrl);
    NVS.setInt("rs", rainSensor ? 1 : 0);
    NVS.setInt("bs", batterySensor ? 1 : 0);
    NVS.commit();
}

void Config::reconfigure()
{
    NVS.setInt("X", CONFIG_MODE_RESET);
    NVS.commit();
}

Config *Config::instance()
{
    if (_instance == nullptr)
    {
        _instance = new Config();
    }
    return _instance;
}