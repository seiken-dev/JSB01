#include <iostream>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board.h"

// constant variables
constexpr char log_tag[] = "main";

extern "C" {
void app_main(void)
{
  ESP_LOGI(log_tag, "Booting...");
  xiao_c3.init();
  static bool sw1Stat=false, sw2Stat=false;
  while(1)  {
    if (xiao_c3.readSW(xiao_c3.sw1)) {
      if (sw1Stat == false) {
	ESP_LOGI(log_tag, "Pressed");
	sw1Stat = true;
      }
    }
    else {
      if (sw1Stat) {
      sw1Stat = false;
    }
    }
    vTaskDelay(1);
  }
}
}
