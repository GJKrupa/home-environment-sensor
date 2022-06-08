#include <Arduino.h>
#include "states.h"
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "config.h"
#include "logging.h"
#include "assets.h"

#define CURRENT_CONFIG_SET 2

// Let's be Google
IPAddress myIP(8, 8, 8, 8);
DNSServer *dnsServer;
AsyncWebServer *server;

String message = "Please enter all values";
String status = "primary";

String index_processor(const String& var)
{
    if (var == "STATUS") return status;
    else if (var == "MESSAGE") return message;
    else if (var == "NAME") return Config::instance()->name;
    else if (var == "SSID") return Config::instance()->ssid;
    else if (var == "MQTT_HOST") return Config::instance()->mqttHost;
    else if (var == "MQTT_PORT") return String(Config::instance()->mqttPort);
    else if (var == "SYSLOG_HOST") return Config::instance()->syslogHost;
    else if (var == "SYSLOG_PORT") return String(Config::instance()->syslogPort);
    else if (var == "UPDATE_URL") return Config::instance()->updateUrl;
    else if (var == "RAIN_SENSOR") return Config::instance()->rainSensor ? "checked" : "";
    else if (var == "BATTERY_SENSOR") return Config::instance()->batterySensor ? "checked" : "";
    else return "";
}

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const char*)index_html_start, index_processor);
  }
};

State state_config_check(State state)
{
    Config::instance()->load();

    if (Config::instance()->mode == CONFIG_MODE_UNSET)
    {
        logln("No configuration in NVS. Enabling WiFi AP mode");
        return ST_CONFIG_ACTIVATE;
    }
    else if (Config::instance()->mode == CONFIG_MODE_RESET)
    {
        logln("NVS reconfigure requested. Enabling WiFi AP mode");
        return ST_CONFIG_ACTIVATE;
    }
    else if (Config::instance()->version < CURRENT_CONFIG_SET)
    {
        logln("Old configuration in NVS. Enabling WiFi AP mode");
        return ST_CONFIG_ACTIVATE;
    }
    else
    {
        return ST_FEATURE_ENABLE;
    }
}

State state_config_activate(State state)
{
    server = new AsyncWebServer(80);
    dnsServer = new DNSServer();

    server->onNotFound([](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html_start, index_processor);
    });

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html_start, index_processor);
    });

    server->on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/css", bootstrap_min_css_start);
    });

    server->on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/js", bootstrap_min_js_start);
    });
        
    server->on("/configure", HTTP_POST, [] (AsyncWebServerRequest *request) {
        String inputMessage;
        String inputParam;
        message = "";
        bool failed = false;
    
        if (request->hasParam("name", true))
        {
            Config::instance()->name = request->getParam("name", true)->value();
        }

        if (Config::instance()->name.isEmpty())
        {
            message = message + "You must supply a name for the device<br />";
            failed = true;
        }

        if (request->hasParam("ssid", true))
        {
            Config::instance()->ssid = request->getParam("ssid", true)->value();
        }

        if (Config::instance()->ssid.isEmpty())
        {
            message = message + "You must supply an SSID to connect to<br />";
            failed = true;
        }
        
        if (request->hasParam("passphrase", true))
        {
            inputMessage = request->getParam("passphrase", true)->value();
            if (!inputMessage.isEmpty())
            {
                Config::instance()->passphrase = inputMessage;
            }
        }

        if (Config::instance()->passphrase.isEmpty())
        {
            message = message + "You must supply a passphrase for the network<br />";
            failed = true;
        }

        if (request->hasParam("mqttHost", true))
        {
            Config::instance()->mqttHost = request->getParam("mqttHost", true)->value();
        }

        if (Config::instance()->mqttHost.isEmpty())
        {
            message = message + "You must supply an MQTT host<br />";
            failed = true;
        }

        if (request->hasParam("mqttPort", true))
        {
            inputMessage = request->getParam("mqttPort", true)->value();
            Config::instance()->mqttPort = inputMessage.toInt();
        }

        if (Config::instance()->mqttPort < 1 || Config::instance()->mqttPort > 65535)
        {
            message = message + "You must supply an MQTT port with a value between 1 and 63353<br />";
            failed = true;
        }

        if (request->hasParam("updateUrl", true))
        {
            Config::instance()->updateUrl = request->getParam("updateUrl", true)->value();
        }

        if (request->hasParam("syslogHost", true))
        {
            Config::instance()->syslogHost = request->getParam("syslogHost", true)->value();

            if (request->hasParam("syslogPort", true))
            {
                inputMessage = request->getParam("syslogPort", true)->value();
                Config::instance()->syslogPort = inputMessage.toInt();
            }

            if (Config::instance()->syslogPort < 1 || Config::instance()->syslogPort > 65535)
            {
                message = message + "You must supply an syslog port with a value between 1 and 63353<br />";
                failed = true;
            }
        }
        else
        {
            Config::instance()->syslogHost = "";
            Config::instance()->syslogPort = 514;
        }

        if (request->hasParam("updateUrl", true))
        {
            inputMessage = request->getParam("updateUrl", true)->value();
            Config::instance()->updateUrl = inputMessage;
        }

        Config::instance()->rainSensor = request->hasParam("rainSensor", true);

        Config::instance()->batterySensor = request->hasParam("batterySensor", true);

        if (failed)
        {
            status = "danger";
            request->send_P(200, "text/html", index_html_start, index_processor);
        } 
        else
        {
            request->send(200, "text/html", success_html_start);
            Config::instance()->version = CURRENT_CONFIG_SET;
            Config::instance()->save();
            ESP.restart();
        }
    });

    byte mac[6];
    WiFi.macAddress(mac);
    String ap_name;
    if (Config::instance()->name == "")
    {
        ap_name = 
            String("env-sensor-")
            + String(mac[0],HEX) 
            + String(mac[1],HEX) 
            + String(mac[2],HEX) 
            + String(mac[3],HEX) 
            + String(mac[4],HEX) 
            + String(mac[5],HEX);
    }
    else
    {
        ap_name = String("env-sensor-") + Config::instance()->name;
    }

    WiFi.mode(WIFI_AP); 
    WiFi.softAPConfig(myIP, myIP, IPAddress(255,255,255,0));
    WiFi.softAP(ap_name.c_str());
    
    dnsServer->start(53, "*", myIP);
    server->addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    server->begin();
    logfmt("AP up: %s\n", ap_name.c_str());

    return ST_CONFIG_ACCEPT;
}

State state_config_accept(State state)
{
    dnsServer->processNextRequest();
    return ST_CONFIG_ACCEPT;
}