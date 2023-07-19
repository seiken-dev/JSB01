#include <iostream>
#include "esp_log.h"

// constant variables
constexpr char log_tag[] = "main";

extern "C" {
void app_main(void)
{
  ESP_LOGI(log_tag, "Booting...");
  while(1) ;
}
}
