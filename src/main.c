// #define IOTA_COMMON_EXAMPLE

#ifdef IOTA_COMMON_EXAMPLE

/**
 * For application uses iota_common only
 */
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
  if (tmp_addr == NULL) {
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

#else
/**
 * For application uses iota.c
 */
#include <inttypes.h>
#include <stdio.h>

#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#include "utils/logger_helper.h"
#include "utils/time.h"

// IOTA Node configuration
#define IOTA_NODE_URL "nodes.devnet.iota.org"
#define IOTA_NODE_PORT 443
#define USE_HTTPS 1

// please replace it to your WIFI setting.
#define APP_WIFI_SSID "wifi_ssd"
#define APP_WIFI_PWD "wifi_pwd"

static char const *amazon_ca1_pem =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
    "-----END CERTIFICATE-----\r\n";

static EventGroupHandle_t wifi_event_group;
static const char *TAG = "esp32_main";

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const static int CONNECTED_BIT = BIT0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
  switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
      esp_wifi_connect();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      /* This is a workaround as ESP32 WiFi libs don't currently
             auto-reassociate. */
      esp_wifi_connect();
      xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
      break;
    default:
      break;
  }
  return ESP_OK;
}

static void wifi_conn_init(void) {
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = APP_WIFI_SSID,
              .password = APP_WIFI_PWD,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

static void initialize_nvs() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

void example_node_info(iota_client_service_t *s) {
  printf("\n[%s]\n", __FUNCTION__);
  retcode_t ret = RC_ERROR;
  get_node_info_res_t *node_res = get_node_info_res_new();
  if (node_res == NULL) {
    printf("Error: OOM\n");
    return;
  }

  if ((ret = iota_client_get_node_info(s, node_res)) == RC_OK) {
    printf("appName %s \n", get_node_info_res_app_name(node_res));
    printf("appVersion %s \n", get_node_info_res_app_version(node_res));

    printf("latestMilestone: ");
    flex_trit_print(node_res->latest_milestone, NUM_TRITS_HASH);
    printf("\n");

    printf("latestMilestoneIndex %u \n", node_res->latest_milestone_index);

    printf("latestSolidSubtangleMilestone: ");
    flex_trit_print(node_res->latest_solid_subtangle_milestone, NUM_TRITS_HASH);
    printf("\n");

    printf("latestSolidSubtangleMilestoneIndex %u \n", node_res->latest_solid_subtangle_milestone_index);
    printf("neighbors %d \n", node_res->neighbors);
    printf("packetsQueueSize %d \n", node_res->packets_queue_size);
    printf("time %" PRIu64 " \n", node_res->time);
    printf("tips %d \n", node_res->tips);
    printf("transactionsToRequest %d \n", node_res->transactions_to_request);

    // print out features
    printf("features: ");
    size_t num_features = get_node_info_req_features_num(node_res);
    for (; num_features > 0; num_features--) {
      printf("%s, ", get_node_info_res_features_at(node_res, num_features - 1));
    }
    printf("\n");

    // print out the coordinator address
    printf("coordinatorAddress: ");
    flex_trit_print(node_res->coordinator_address, NUM_TRITS_ADDRESS);
    printf("\n");

  } else {
    printf("Error: %s", error_2_string(ret));
  }

  get_node_info_res_free(&node_res);
}

void app_main() {
  // delay for console
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  initialize_nvs();
  // init wifi
  wifi_conn_init();

  ESP_LOGI(TAG, "Connecting to WiFi network...");
  /* Wait for the callback to set the CONNECTED_BIT in the event group. */
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

  printf("Hello world! ESP-IDF %s\n", esp_get_idf_version());
  iota_client_service_t *serv = iota_client_core_init(IOTA_NODE_URL, IOTA_NODE_PORT, USE_HTTPS ? amazon_ca1_pem : NULL);
  if (serv == NULL) {
    printf("OOM\n");
    goto end;
  }
  logger_helper_init(LOGGER_DEBUG);
  logger_init_client_core(LOGGER_DEBUG);
  logger_init_client_extended(LOGGER_DEBUG);
  logger_init_json_serializer(LOGGER_DEBUG);

  ESP_LOGI(TAG, "Connected to AP");
  ESP_LOGI(TAG, "IRI Node: %s, port: %d\n", serv->http.host, serv->http.port);
  printf("Connecting to node: https://%s:%u\n", serv->http.host, serv->http.port);
  example_node_info(serv);
  iota_client_core_destroy(&serv);

  // cleanup logger
  logger_destroy_client_core();
  logger_destroy_client_extended();
  logger_destroy_json_serializer();
  logger_helper_destroy();

end:
  for (int i = 20; i >= 0; i--) {
    printf("Restarting in %d seconds...\n", i);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  printf("Restarting now.\n");

  fflush(stdout);
  esp_restart();
}
#endif