#include "pico/stdio.h"
#include <iostream>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "hardware/pwm.h"

void gpioInit(uint n, bool dir) {
  gpio_init(n);
  gpio_set_dir(n, dir);
  if (dir == GPIO_IN) gpio_pull_up(n);
}
constexpr uint sw = 2;
int main(){
  gpioInit(sw, GPIO_IN);
  while(1) {
    if (gpio_get(sw) == false) {
      // Pushed
    }
    sleep_ms(10);
  }
  return 0;
}
