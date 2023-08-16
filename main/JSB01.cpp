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
  while(1)  {
    vTaskDelay(1000);
  }
}
}
