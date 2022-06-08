#pragma once
#include <Arduino.h>

void log_init(String name);
void log_close();
void logln(const char* line);
void logfmt(const char *format, ...);