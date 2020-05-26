#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <BME280_t.h>
#include <readings.h>

// General configuration
#define MICROSECONDS_IN_SECOND 1000000L
#define FAILURE_BACKIFF 5L * MICROSECONDS_IN_SECOND
#define REPORT_PERIOD 120L * MICROSECONDS_IN_SECOND
#define PASCALS_IN_HPA 100.0f
#define MAX_VOLTAGE 4.4 * 5.0

// Instance-specific configuration
#define ALTITUDE 80.0f
#define ROOM "garden"
#define RAIN_MONITOR 1

RTC_DATA_ATTR double lastExecutionTime;

BME280<> bme;
esp_sleep_wakeup_cause_t wakeupCause;
long executionStart;

Readings *readings;

void goToSleep(long microSeconds) {
#ifdef RAIN_MONITOR
    digitalWrite(27, LOW);
#endif
    digitalWrite(32, LOW);
    Serial.flush();
    esp_sleep_enable_timer_wakeup(microSeconds);
    esp_deep_sleep_start();
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

void setup(void)
{
    executionStart = esp_timer_get_time();
    wakeupCause = esp_sleep_get_wakeup_cause();
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
        goToSleep(FAILURE_BACKIFF);
    }

    readings = new Readings(ROOM);
    bme.refresh();
    readings->setValue(BATTERY_UUID, toBatteryVoltage(analogRead(33)));
    readings->setValue(TEMPERATURE_UUID, bme.temperature);
    readings->setValue(HUMIDITY_UUID, bme.humidity);
    readings->setValue(PRESSURE_UUID, bme.seaLevelForAltitude(ALTITUDE) / PASCALS_IN_HPA);
    #ifdef RAIN_MONITOR
        readings->setValue(RAIN_UUID, toRainReading(analogRead(34)));
        digitalWrite(27, LOW);
    #endif
    readings->start();
}

void loop(void)
{
    if (readings->readCompleted()) {
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
    } else if (esp_timer_get_time() - executionStart > 10L * MICROSECONDS_IN_SECOND) {
        long executionTime = esp_timer_get_time() - executionStart;
        lastExecutionTime = (double)executionTime;
        long sleepTime = REPORT_PERIOD - executionTime;
        double activeRatio = ((double)executionTime) / ((double)REPORT_PERIOD);

        Serial.printf("NO CONTACT FROM CENTRAL! - Execution took %ld us, sleeping for %ld, active for %.2f%% of the time\n",
            executionTime,
            sleepTime,
            activeRatio * 100.0
        );

        goToSleep(sleepTime);
    }
}