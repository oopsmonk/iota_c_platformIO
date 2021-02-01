#include <inttypes.h>
#include <stdio.h>
#include <unity.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sys/time.h"

#include "core/address.h"

#define ADDR_PATH "m/44'/4128'/0'/0'"
#define ADDR_PATH_MAX 64
#define ADDR_NUMS 100

static char* get_path_from_index(uint32_t index) {
  int ret_size = 0;
  char* path = malloc(ADDR_PATH_MAX);
  if (path) {
    // Bip44 Paths: m/44'/4128'/Account'/Change'/Index'
    ret_size = snprintf(path, ADDR_PATH_MAX, "%s/%" PRIu32 "'", ADDR_PATH, index);
    if (ret_size >= ADDR_PATH_MAX) {
      path[ADDR_PATH_MAX - 1] = '\0';
    }
  }
  return path;
}

static int64_t time_in_us() {
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  return (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
};

void bench_addr_gen() {
  int err = 0;
  // get a random seed
  byte_t seed[IOTA_SEED_BYTES];
  byte_t addr[ED25519_ADDRESS_BYTES];
  iota_crypto_randombytes(seed, IOTA_SEED_BYTES);

  char* tmp_path = NULL;
  printf("Bench address generation: %d times\n\t\tmin(ms)\tmax(ms)\tavg(ms)\ttotal(ms)\n", ADDR_NUMS);
  int64_t time_spent = 0;
  int64_t min = 0, max = 0, sum = 0;

  for (uint32_t idx = 0; idx < ADDR_NUMS; idx++) {
    tmp_path = get_path_from_index(idx);
    if (tmp_path) {
      int64_t curr_t = time_in_us();
      // generating address
      err = address_from_path(seed, tmp_path, addr);
      time_spent = time_in_us() - curr_t;
      free(tmp_path);
      tmp_path = NULL;
      if (err) {
        printf("genrate addr failed at index %" PRIu32 ", seed: ", idx);
        dump_hex_str(seed, IOTA_SEED_BYTES);
        break;
      }
      // printf("time spent: %lld us\n", time_spent);
      max = (idx == 0 || time_spent > max) ? time_spent : max;
      min = (idx == 0 || time_spent < min) ? time_spent : min;
      sum += time_spent;
    } else {
      printf("get path from index failed\n");
      break;
    }
  }
  printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", (min / 1000.0), (max / 1000.0), (sum / ADDR_NUMS) / 1000.0, sum / 1000.0);
}

void app_main() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  UNITY_BEGIN();

  printf("ESP-IDF version %s\n", esp_get_idf_version());
  RUN_TEST(bench_addr_gen);

  UNITY_END();
}
