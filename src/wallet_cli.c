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

/* 'addresses' command */
static int fn_addresses(int argc, char **argv) {
  char addr_str[TANGLE_ADDRESS_BASE58_BUF];
  addr_list_t *addrs = am_addresses(w_ctx->addr_manager);
  address_t *elm = NULL;
  ADDR_LIST_FOREACH(addrs, elm) {
    address_2_base58(elm->addr, addr_str);
    printf("[%" PRIu64 "] %s %s\n", elm->index, addr_str,
           am_is_spent_address(w_ctx->addr_manager, elm->index) ? "[spent]" : "");
  }
  return 0;
}

static void register_addresses() {
  const esp_console_cmd_t addr_cmd = {
      .command = "addresses",
      .help = "list all addresses",
      .hint = NULL,
      .func = &fn_addresses,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&addr_cmd));
}

/* 'wallet_info' command */
static int fn_wallet_info(int argc, char **argv) {
  wallet_refresh(w_ctx, true);
  wallet_status_print(w_ctx);
  return 0;
}

static void register_wallet_info() {
  const esp_console_cmd_t cmd = {
      .command = "wallet_info",
      .help = "Shows wallet information",
      .hint = NULL,
      .func = &fn_wallet_info,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'send_funds' command */
static struct {
  struct arg_str *receiver;
  struct arg_str *value;
  struct arg_end *end;
} send_funds_args;

static int fn_send_funds(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&send_funds_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, send_funds_args.end, argv[0]);
    return -1;
  }

  // check parameters
  char const *receiver = send_funds_args.receiver->sval[0];
  char *endptr = NULL;
  int64_t value = strtoll(send_funds_args.value->sval[0], &endptr, 10);
  if (value <= 0) {
    printf("Error: funds <= 0\n");
    return -1;
  }

  send_funds_op_t send_op = {};
  send_op.amount = value;
  if (address_from_base58(receiver, send_op.receiver) == false) {
    printf("Invalid receiver address\n");
    return -1;
  }
  return wallet_send_funds(w_ctx, &send_op);
}

static void register_send_funds() {
  send_funds_args.receiver = arg_str1(NULL, NULL, "<RECEIVER>", "A receiver address in base58 encoding");
  send_funds_args.value = arg_str0("v", "value", "<VALUE>", "A token value");
  send_funds_args.end = arg_end(2);

  const esp_console_cmd_t cmd = {
      .command = "send_funds",
      .help = "Issue a payment.",
      .hint = NULL,
      .func = &fn_send_funds,
      .argtable = &send_funds_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'req_funds' command */
static int fn_req_funds(int argc, char **argv) {
  wallet_request_funds(w_ctx);
  return 0;
}

static void register_req_funds() {
  const esp_console_cmd_t cmd = {
      .command = "req_funds",
      .help = "Request funds from the facet",
      .hint = NULL,
      .func = &fn_req_funds,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'seed' command */
static int fn_seed(int argc, char **argv) {
  char seed_str[TANGLE_SEED_BASE58_BUF];
  if (seed_2_base58(w_ctx->addr_manager->seed, seed_str) == true) {
    printf("%s\n", seed_str);
  } else {
    printf("Gets seed failed\n");
  }
  return 0;
}

static void register_seed() {
  const esp_console_cmd_t cmd = {
      .command = "seed",
      .help = "Shows the seed of this wallet",
      .hint = NULL,
      .func = &fn_seed,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'new_address' command */
static int fn_new_address(int argc, char **argv) {
  byte_t addr[TANGLE_ADDRESS_BYTES];
  char addr_str[TANGLE_ADDRESS_BASE58_BUF];
  am_get_new_address(w_ctx->addr_manager, addr);
  if (address_2_base58(addr, addr_str) == true) {
    printf("%s\n", addr_str);
  } else {
    printf("Converting address to base58 failed\n");
  }
  return 0;
}

static void register_new_address() {
  const esp_console_cmd_t cmd = {
      .command = "new_address",
      .help = "Gest a new address",
      .hint = NULL,
      .func = &fn_new_address,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'save' command */
static int fn_save(int argc, char **argv) {
  printf("TODO\n");
  return 0;
}

static void register_save() {
  const esp_console_cmd_t cmd = {
      .command = "save",
      .help = "Write wallet settings",
      .hint = NULL,
      .func = &fn_save,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

//============= Public functions====================

void register_wallet_commands() {
  // system info
  register_free();
  register_heap();
  register_stack_info();
  register_version();
  register_restart();

  // wallet APIs
  register_node_info();
  register_balance();
  register_addresses();
  register_wallet_info();
  register_send_funds();
  register_req_funds();
  register_seed();
  register_new_address();
  register_save();
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
