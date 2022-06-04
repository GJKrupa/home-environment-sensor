#pragma once
#include <Arduino.h>

void log_init();
void log_close();
void logln(const char* line);
void logf(const char *format, ...);