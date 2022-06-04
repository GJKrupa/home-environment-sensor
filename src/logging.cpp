#ifndef SYSLOG_ENABLED
#define SYSLOG_ENABLED 1
#endif

#ifndef SYSLOG_HOST
#define SYSLOG_HOST "localhost"
#endif

#ifndef SYSLOG_PORT
#define SYSLOG_PORT 514
#endif

#include "logging.h"
#include <Arduino.h>
#include <WiFi.h>

#if SYSLOG_ENABLED == 1
#include <Syslog.h>
#endif

#if SYSLOG_ENABLED == 1
WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_PROTO_BSD);
#endif

void log_init()
{
    Serial.begin(115200);
#if SYSLOG_ENABLED == 1
    Serial.printf("Sending logs to %s:%d\n", SYSLOG_HOST, SYSLOG_PORT);
    syslog.server(SYSLOG_HOST, SYSLOG_PORT);
    syslog.deviceHostname(ROOM);
    syslog.appName("environment");
    syslog.defaultPriority(LOG_KERN);
    syslog.logMask(LOG_UPTO(LOG_INFO));
#endif
}

void log_close()
{
    Serial.flush();
}

void logln(const char* line)
{
    Serial.println(line);
#if SYSLOG_ENABLED == 1
    if (WiFi.isConnected())
    {
        Serial.printf("Syslog...");
        syslog.log(LOG_INFO, line);
    }
#endif
}

void logf(const char *format, ...)
{
    char buf[255];
    va_list arg;
    va_list copy1;
    va_list copy2;
    va_start(arg, format);
    va_copy(copy1, arg);
    vsprintf(buf, format, copy1);
    Serial.print(buf);
    va_end(copy1);
#if SYSLOG_ENABLED == 1
    if (WiFi.isConnected())
    {
        Serial.printf("Syslog...");
        va_copy(copy2, arg);
        syslog.vlogf(LOG_INFO, format, copy2);
        va_end(copy2);
    }
#endif
    va_end(arg);
}