#include "logging.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Syslog.h>
#include "config.h"

WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_PROTO_BSD);



void log_init(String name)
{
    if (Config::instance()->syslogEnabled)
    {
        Serial.printf("Sending logs to %s:%d\n", Config::instance()->syslogHost.c_str(), Config::instance()->syslogPort);
        syslog.server(Config::instance()->syslogHost.c_str(), Config::instance()->syslogPort);
        syslog.deviceHostname(Config::instance()->name.c_str());
        syslog.appName("environment");
        syslog.defaultPriority(LOG_KERN);
        syslog.logMask(LOG_UPTO(LOG_INFO));
    }
}

void log_close()
{
    Serial.flush();
}

void logln(const char* line)
{
    Serial.println(line);
    if (Config::instance()->syslogEnabled && WiFi.isConnected())
    {
        syslog.log(LOG_INFO, line);
    }
}

void logfmt(const char *format, ...)
{
    char buf[255];
    va_list arg;
    va_list copy1;
    va_start(arg, format);
    va_copy(copy1, arg);
    vsprintf(buf, format, copy1);
    Serial.print(buf);
    va_end(copy1);

    if (Config::instance()->syslogEnabled && WiFi.isConnected())
    {
        va_list copy2;
        va_copy(copy2, arg);
        syslog.vlogf(LOG_INFO, format, copy2);
        va_end(copy2);
    }

    va_end(arg);
}