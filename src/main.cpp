#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <BME280_t.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <HTTPClient.h>

// General configuration
#define MICROSECONDS_IN_SECOND 1000000L
#define WIFI_FAILURE_BACKOFF 5L * MICROSECONDS_IN_SECOND
#define REPORT_PERIOD 120L * MICROSECONDS_IN_SECOND
#define PASCALS_IN_HPA 100.0f
#define MAX_VOLTAGE 4.4 * 5.0

// Instance-specific configuration
#define RAIN_MONITOR 1
#define ALTITUDE 80.0f
#define ROOM "garden"
#define SSID "***SSID HERE OR USE CFLAGS***"
#define PASSCODE "***WIFI PASSCODE HERE OR USE CFLAGS***"
#define TELEGRAF_ENDPOINT "http://tick-stack.localdomain:8186/write"
#define TIME_SERVER "uk.pool.ntp.org"

RTC_DATA_ATTR double lastExecutionTime;

BME280<> bme;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, TIME_SERVER, 0, 60000);
HTTPClient http;
long executionStart;
esp_sleep_wakeup_cause_t wakeupCause;

void goToSleep(long microSeconds) {
#ifdef RAIN_MONITOR
    digitalWrite(27, LOW);
#endif
    digitalWrite(32, LOW);
    Serial.flush();
    esp_sleep_enable_timer_wakeup(microSeconds);
    esp_deep_sleep_start();
}

void setup(void)
{
    wakeupCause = esp_sleep_get_wakeup_cause();
    executionStart = esp_timer_get_time();
    pinMode(33, INPUT);
    pinMode(32, OUTPUT);
#ifdef RAIN_MONITOR
    pinMode(34, INPUT);
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
#endif
    digitalWrite(32, HIGH);
    Wire.begin(25,26);
    Serial.begin(115200);
    bool status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        goToSleep(WIFI_FAILURE_BACKOFF);
    }

    WiFi.begin(SSID, PASSCODE);
    int count = 0;
    while ( WiFi.status() != WL_CONNECTED && count++ < 60) {
        Serial.printf("Connecting: %d\n", count);
        delay ( 500 );
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected");
    } else {
        Serial.println("Failed to connect to WiFi");
        goToSleep(WIFI_FAILURE_BACKOFF);
    }

    Serial.println("Getting time from NTP");
    timeClient.update();
    Serial.println("Time updated from NTP");
}

void sendReading(const char* const name, const double value) {
    char buf[128];
    sprintf(buf, "%s,room=%s value=%.2f %ld000000000", name, ROOM, value, timeClient.getEpochTime());
    Serial.printf("Sending %s\n", buf);
    http.begin(TELEGRAF_ENDPOINT);
    int code = http.POST((uint8_t*)buf, strlen(buf));
    Serial.printf("Response: code=%d, message=\"%s\"\n", code, http.getString().c_str());
    http.end();
}

double toBatteryVoltage(int analogueReading) {
    return ((double)analogueReading / 4095.0) * MAX_VOLTAGE;
}

#ifdef RAIN_MONITOR
double toRainReading(int analogueReading) {
    int invertedReading = 4095 - analogueReading;
    return ((double)invertedReading / 4095.8) * 100.0;
}
#endif

void loop(void)
{
    bme.refresh();
    if (wakeupCause == ESP_SLEEP_WAKEUP_TIMER) {
        sendReading("lastExecutionTime", lastExecutionTime / 1000000.0);
    }
    sendReading("battery", toBatteryVoltage(analogRead(33)));
    sendReading("temperature", bme.temperature);
    sendReading("humidity", bme.humidity);
    sendReading("pressure", bme.seaLevelForAltitude(ALTITUDE) / PASCALS_IN_HPA);
#ifdef RAIN_MONITOR
    sendReading("rain", toRainReading(analogRead(34)));
    pinMode(34, INPUT);
#endif

    long executionTime = esp_timer_get_time() - executionStart;
    lastExecutionTime = (double)executionTime;
    long sleepTime = REPORT_PERIOD - executionTime;
    double activeRatio = ((double)executionTime) / ((double)REPORT_PERIOD);

    Serial.printf("Execution took %ld us, sleeping for %ld, active for %.2f%% of the time\n",
        executionTime,
        sleepTime,
        activeRatio * 100.0
    );

    goToSleep(sleepTime);
}