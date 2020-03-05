#include <stdio.h>
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"

void app_main() {
    char addr[NUM_TRYTES_ADDRESS + 1] = {};
  // delay for console
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  printf("Hello world! ESP-IDF %s\n", esp_get_idf_version());
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  printf("SEED: %s\n", seed);
  char *tmp_addr = iota_sign_address_gen_trytes(seed, 0, 2);
  if(tmp_addr == NULL){
      printf("gen address failed\n");
      goto end;
  }
  memcpy(addr, tmp_addr, NUM_TRYTES_ADDRESS);
  addr[NUM_TRYTES_ADDRESS] = '\0';
  printf("ADDRESS: %s\n", addr);
  free(tmp_addr);

end:
  for (int i = 20; i >= 0; i--) {
    printf("Restarting in %d seconds...\n", i);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  printf("Restarting now.\n");

  fflush(stdout);
  esp_restart();
}
