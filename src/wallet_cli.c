#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "argtable3/argtable3.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "esp32/rom/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"

#include "http_parser.h"
#include "wallet_cli.h"

#include "client/api/get_funds.h"
#include "client/api/get_node_info.h"

static const char *TAG = "wallet_cli";

static wallet_t *w_ctx = NULL;

// 0 on success
static int config_validation() {
  // URL parsing
  struct http_parser_url u;
  http_parser_url_init(&u);
  int parse_ret = http_parser_parse_url(CONFIG_NODE_ENDPOINT, strlen(CONFIG_NODE_ENDPOINT), 0, &u);
  if (parse_ret != 0) {
    ESP_LOGE(TAG, "validating endpoint error\n");
    return -1;
  }

  // seed length
  if (strcmp(CONFIG_WALLET_SEED, "random") != 0) {
    if (strlen(CONFIG_WALLET_SEED) < 43) {
      ESP_LOGE(TAG, "validating seed error\n");
      return -1;
    }
  }
  return 0;
}

/* 'version' command */
static int fn_get_version(int argc, char **argv) {
  esp_chip_info_t info;
  esp_chip_info(&info);
  printf("IDF Version:%s\r\n", esp_get_idf_version());
  printf("Chip info:\r\n");
  printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
  printf("\tcores:%d\r\n", info.cores);
  printf("\tfeature:%s%s%s%s%d%s\r\n", info.features & CHIP_FEATURE_WIFI_BGN ? ", 802.11bgn" : "",
         info.features & CHIP_FEATURE_BLE ? ", BLE" : "", info.features & CHIP_FEATURE_BT ? ", BT" : "",
         info.features & CHIP_FEATURE_EMB_FLASH ? ", Embedded-Flash:" : ", External-Flash:",
         spi_flash_get_chip_size() / (1024 * 1024), " MB");
  printf("\trevision number:%d\r\n", info.revision);
  printf("APP_VERSION: %s\n", APP_WALLET_VERSION);
  return 0;
}

static void register_version() {
  const esp_console_cmd_t cmd = {
      .command = "version",
      .help = "Get version of chip and wallet",
      .hint = NULL,
      .func = &fn_get_version,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'restart' command */
static int fn_restart(int argc, char **argv) {
  ESP_LOGI(TAG, "Restarting");
  wallet_free(w_ctx);
  w_ctx = NULL;
  esp_restart();
}

static void register_restart() {
  const esp_console_cmd_t cmd = {
      .command = "restart",
      .help = "Software reset of the chip",
      .hint = NULL,
      .func = &fn_restart,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'free' command */
static int fn_free_mem(int argc, char **argv) {
  printf("%d\n", esp_get_free_heap_size());
  return 0;
}

static void register_free() {
  const esp_console_cmd_t cmd = {
      .command = "free",
      .help = "Get the current size of free heap memory",
      .hint = NULL,
      .func = &fn_free_mem,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'heap' command */
static int fn_heap_size(int argc, char **argv) {
  printf("heap info (SPI RAM): \n");
  heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
  printf("\nheap info (DEFAULT): \n");
  heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
  return 0;
}

static void register_heap() {
  const esp_console_cmd_t heap_cmd = {
      .command = "heap",
      .help = "Get heap memory info",
      .hint = NULL,
      .func = &fn_heap_size,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&heap_cmd));
}

/* 'stack' command */
static int fn_stack_info(int argc, char **argv) {
  printf("%u tasks are running on the system\n", uxTaskGetNumberOfTasks());
  printf("Main stack size: %d, remaining %d bytes\n", CONFIG_MAIN_TASK_STACK_SIZE, uxTaskGetStackHighWaterMark(NULL));
  return 0;
}

static void register_stack_info() {
  const esp_console_cmd_t stack_info_cmd = {
      .command = "stack",
      .help = "Get system stack info",
      .hint = NULL,
      .func = &fn_stack_info,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&stack_info_cmd));
}

/* 'node_info' command */
static int fn_node_info(int argc, char **argv) {
  res_node_info_t info;
  int ret = get_node_info(&w_ctx->endpoint, &info);
  if (ret == 0) {
    printf("Node: %s is %s\n", w_ctx->endpoint.url, info.is_synced ? "synced" : "unsynced");
    printf("ID: %s Version: %s\n", info.id, info.version);
  } else {
    printf("Error: get node info failed\n");
  }
  return 0;
}

static void register_node_info() {
  const esp_console_cmd_t node_info_cmd = {
      .command = "node_info",
      .help = "Get node info",
      .hint = NULL,
      .func = &fn_node_info,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&node_info_cmd));
}

/* 'balance' command */
static int fn_balance(int argc, char **argv) {
  printf("Wallet balance: %" PRIu64 "\n", wallet_balance(w_ctx));
  return 0;
}

static void register_balance() {
  const esp_console_cmd_t balance_cmd = {
      .command = "balance",
      .help = "Display wallet balance",
      .hint = NULL,
      .func = &fn_balance,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&balance_cmd));
}

//============= Public functions====================

void register_wallet_commands() {
  // system info
  register_free();
  register_heap();
  register_stack_info();
  register_version();
  register_restart();

  // cclient APIs
  register_node_info();
  register_balance();
}

int init_wallet() {
  byte_t seed[TANGLE_SEED_BYTES];

  if (config_validation() != 0) {
    return -1;
  }

  if (strcmp(CONFIG_WALLET_SEED, "random") == 0) {
    random_seed(seed);
  } else {
    seed_from_base58(CONFIG_WALLET_SEED, seed);
  }

  w_ctx = wallet_init(CONFIG_NODE_ENDPOINT, CONFIG_NODE_PORT, seed, CONFIG_WALLET_LAST_ADDR,
                      CONFIG_WALLET_FIRST_UNSPENT, CONFIG_WALLET_LAST_UNSPENT);
  if (w_ctx == NULL) {
    printf("wallet create instance failed\n");
    return -1;
  }
  wallet_status_print(w_ctx);
  bool synced = wallet_is_node_synced(w_ctx);
  printf("Is endpoint synced? %s\n", synced ? "Yes" : "No");
  printf("balance: %" PRIu64 "\n", wallet_balance(w_ctx));
  return 0;
}
