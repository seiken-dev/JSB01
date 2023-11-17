// https://github.com/m5stack/M5-DLight
// modified by https://github.com/vcraftjp 2023/08/28

#ifndef __M5_BH1750FVI_H__
#define __M5_BH1750FVI_H__

#include "Arduino.h"
#include "Wire.h"
// #include "pins_arduino.h"

#define BH1750FVI_ADDR 0x23

// CMD
#define POWER_DOWN                      0b00000000
#define POWER_ON                        0b00000001
#define RESET                           0b00000111
#define CONTINUOUSLY_H_RESOLUTION_MODE  0b00010000
#define CONTINUOUSLY_H_RESOLUTION_MODE2 0b00010001
#define CONTINUOUSLY_L_RESOLUTION_MODE  0b00010011
#define ONE_TIME_H_RESOLUTION_MODE      0b00100000
#define ONE_TIME_H_RESOLUTION_MODE2     0b00100001
#define ONE_TIME_L_RESOLUTION_MODE      0b00100011

class M5_BH1750FVI {
   private:
    TwoWire *_wire;
//  uint8_t _sda;
//  uint8_t _scl;
//  uint32_t _freq;
    uint8_t _addr;
    void writeByte(byte cmd);
    void writeBytes(uint8_t *buffer, size_t size);
    void readBytes(uint8_t *buffer, size_t size);

   public:
    M5_BH1750FVI(uint8_t addr = BH1750FVI_ADDR);
//  void begin(TwoWire *wire = &Wire, uint8_t sda = SDA, uint8_t scl = SCL,
//             uint32_t freq = 4000000UL);
    bool begin(TwoWire *wire = &Wire, uint8_t sda = SDA, uint8_t scl = SCL,
             uint32_t freq = 400000);

    void powerOn();
    void powerOff();
    void powerReset();
    void setMode(byte mode);
    uint16_t getLUX();
};

#endif