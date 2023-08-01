#pragma once

#ifdef DEBUG_SERIAL

#define Serial_begin() Serial.begin(115200)
#define Serial_print(v) Serial.print(v)
#define Serial_println(v) Serial.println(v)
#define Serial_printf(...) Serial.printf(__VA_ARGS__)

#else

#define Serial_begin()
#define Serial_print(v)
#define Serial_println(v)
#define Serial_printf(...)

#endif
