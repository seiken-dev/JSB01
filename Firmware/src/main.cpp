#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>

#include <iostream>

#include "hardware/pwm.h"
#include "pico/stdio.h"

void gpioInit(uint n, bool dir) {
  gpio_init(n);
  gpio_set_dir(n, dir);
  if (dir == GPIO_IN) gpio_pull_up(n);
}

constexpr uint sw = 2;
int main() {
  stdio_init_all();
  for (;;) {
    std::cout << "Hello world" << std::endl;
    sleep_ms(2000);
  }
  return 0;
}
