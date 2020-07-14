#include <Arduino.h>

#define IOTA_COMMON_EXAMPLE

#ifdef IOTA_COMMON_EXAMPLE
/**
 * For application uses iota_common only
 */

#define NEW_COMMON

#ifndef NEW_COMMON
#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"
#else
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "model/address.h"
#include "model/transaction.h"
#endif

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

#ifdef NEW_COMMON
TaskHandle_t Task1_Handler;

void gen_addr(void * pvParameters){
  tryte_t addr[NUM_TRYTES_ADDRESS];
  // put your main code here, to run repeatedly:
  Serial.println("Hello ESP32 Arduino!");
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  Serial.print("SEED: ");
  Serial.println(seed);
  generate_address_trytes((tryte_t *)seed, 0, ADDR_SECURITY_LOW, addr);
  Serial.print("Address: ");
  Serial.println((char *)addr);

}
void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello ESP32 Arduino!");
  // xTaskCreatePinnedToCore(gen_addr, "gen_addr", 81920, NULL, 1, &Task1_Handler, 1);
  xTaskCreatePinnedToCore(gen_addr, "gen_addr", 81920, NULL, 1, NULL, 1);
  delay(10000);
}
#else
void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello ESP32 Arduino!");
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  Serial.print("SEED: ");
  Serial.println(seed);
  char *addr = iota_sign_address_gen_trytes(seed, 0, 2);
  Serial.print("Address: ");
  Serial.println(addr);
  free(addr);
  delay(10000);
}
#endif

#else
/**
 * For application uses iota.c
 */
#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"

#include <WiFi.h>

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

void example_node_info(iota_client_service_t *s) {
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Connecting to ");
    Serial.println(APP_WIFI_SSID);

    WiFi.begin(APP_WIFI_SSID, APP_WIFI_PWD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello ESP32 Arduino!");
  iota_client_service_t *serv = iota_client_core_init(IOTA_NODE_URL, IOTA_NODE_PORT, USE_HTTPS ? amazon_ca1_pem : NULL);
  if (serv == NULL) {
    Serial.println("OOM\n");
    goto end;
  }
  logger_helper_init(LOGGER_DEBUG);
  logger_init_client_core(LOGGER_DEBUG);
  logger_init_client_extended(LOGGER_DEBUG);
  logger_init_json_serializer(LOGGER_DEBUG);

  Serial.print("Connecting to node: https://");
  Serial.print(serv->http.host);
  Serial.print(":");
  Serial.println(serv->http.port);

  example_node_info(serv);
  iota_client_core_destroy(&serv);

  // cleanup logger
  logger_destroy_client_core();
  logger_destroy_client_extended();
  logger_destroy_json_serializer();
  logger_helper_destroy();

end:
  delay(10000);
}

#endif