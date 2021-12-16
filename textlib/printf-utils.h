#pragma once

#include <stdarg.h>

char*  sprintf_to_new_buffer(const char* format, ...);
char* vsprintf_to_new_buffer(const char* format, va_list args);
