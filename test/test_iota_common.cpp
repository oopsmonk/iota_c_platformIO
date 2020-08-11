#include <Arduino.h>
#include <unity.h>

#define NEW_COMMON
#define _SHIMMER_ 

#ifndef NEW_COMMON
#include "common/crypto/kerl/kerl.h"
#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"
#include "test_iota_common.hpp"
#include "utils/input_validators.h"
#else
#include "model/address.h"
#include "model/transaction.h"
#endif

#ifdef _SHIMMER_
#include "address_ed25519.h"
#endif

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}
#ifndef NEW_COMMON
void test_is_trytes() {
  tryte_t *hash0 = (tryte_t *)"XUERGHWTYRTFUYKFKXURKHMFEVLOIFTTCNTXOGLDPCZ9CJLKHROOPGNAQYFJEPGK9OKUQROUECBAVNXRX";
  TEST_ASSERT(is_trytes(hash0, 81) == true);
  // length is unmatched
  TEST_ASSERT(is_trytes(hash0, 20) == false);

  // hash with lowercases
  tryte_t *hash1 = (tryte_t *)"XUERGHWTYRTFUYKFKXURKHMFEVLOIFTTCNTXOGLDPCZ9cjlkhroopgnaqyfjepgk9okuqrouecbavnxrx";
  TEST_ASSERT(is_trytes(hash1, 81) == false);

  // hash with invalid tryte
  tryte_t *hash2 = (tryte_t *)"XUERGH3TYRTFUYKFKXURKHMF5VLOIFTTCNTXOGLDPCZ9CJLKHR0OPGNAQYFJEPGK9OKUQROUECBAVNXRX";
  TEST_ASSERT(is_trytes(hash2, 81) == false);
}

void test_one_absorb(void) {
  const char *expected = "EJEAOOZYSAWFPZQESYDHZCGYNSTWXUMVJOVDWUNZJXDGWCLUFGIMZRMGCAZGKNPLBRLGUNYWKLJTYEAQX";
  char trytes[NUM_TRYTES_HASH] = {};
  memcpy(trytes, "EMIDYNHBWMBCXVDEFOFWINXTERALUKYYPPHKP9JJFGJEIUY9MUDVNFZHMMWZUYUSWAIOWEVTHNWMHANBH", NUM_TRYTES_HASH);

  trit_t trits[NUM_TRITS_HASH];
  Kerl kerl;

  trytes_to_trits((tryte_t *)trytes, trits, NUM_TRYTES_HASH);

  kerl_init(&kerl);

  kerl_absorb(&kerl, trits, NUM_TRITS_HASH);
  kerl_squeeze(&kerl, trits, NUM_TRITS_HASH);

  trits_to_trytes(trits, (tryte_t *)trytes, NUM_TRITS_HASH);

  TEST_ASSERT_EQUAL_MEMORY(expected, trytes, NUM_TRYTES_HASH);
}

void test_gen_addresses(void) {
  int num_addrs = 3;
  char addr[NUM_TRYTES_ADDRESS + 1] = {};
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  printf("Test SEED: %s\n", seed);
  addr[NUM_TRYTES_ADDRESS] = '\0';

  uint32_t counter = 0;
  printf("Security = 1\n");
  for (int i = 0; i < num_addrs; i++) {
    uint32_t curr_t = micros();
    // get addresses from security level 1.
    char *tmp_addr = iota_sign_address_gen_trytes(seed, i, 1);
    counter = micros() - curr_t;
    TEST_ASSERT_NOT_NULL(tmp_addr);
    memcpy(addr, tmp_addr, NUM_TRYTES_ADDRESS);
    printf("[%d] %s\n", i, addr);
    printf("time spent: %d us\n", counter);
    TEST_ASSERT_EQUAL_MEMORY(expected_addr1[i], addr, NUM_TRYTES_HASH);
    free(tmp_addr);
  }

  printf("Security = 2\n");
  for (int i = 0; i < num_addrs; i++) {
    uint32_t curr_t = micros();
    // get addresses from security level 2.
    char *tmp_addr = iota_sign_address_gen_trytes(seed, i, 2);
    counter = micros() - curr_t;
    TEST_ASSERT_NOT_NULL(tmp_addr);
    memcpy(addr, tmp_addr, NUM_TRYTES_ADDRESS);
    printf("[%d] %s\n", i, addr);
    printf("time spent: %d us\n", counter);
    TEST_ASSERT_EQUAL_MEMORY(expected_addr2[i], addr, NUM_TRYTES_HASH);
    free(tmp_addr);
  }

  printf("Security = 3\n");
  for (int i = 0; i < num_addrs; i++) {
    uint32_t curr_t = micros();
    // get addresses from security level 3.
    char *tmp_addr = iota_sign_address_gen_trytes(seed, i, 3);
    counter = micros() - curr_t;
    TEST_ASSERT_NOT_NULL(tmp_addr);
    memcpy(addr, tmp_addr, NUM_TRYTES_ADDRESS);
    printf("[%d] %s\n", i, addr);
    printf("time spent: %d us\n", counter);
    TEST_ASSERT_EQUAL_MEMORY(expected_addr3[i], addr, NUM_TRYTES_HASH);
    free(tmp_addr);
  }
}

#define NUM_OF_TIMES 10

void bench_gen_address(uint8_t security){
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  long run_time = 0;
  long min = 0, max = 0, sum = 0;

  for (int i = 0; i < NUM_OF_TIMES; i++) {
    uint32_t curr_t = micros();
    char *tmp_addr = iota_sign_address_gen_trytes(seed, i, security);
    run_time = micros() - curr_t;
    max = (i == 0 || run_time > max) ? run_time : max;
    min = (i == 0 || run_time < min) ? run_time : min;
    sum += run_time;
    free(tmp_addr);
  }
  printf("security %d:\t%.3f\t%.3f\t%.3f\t%.3f\n", security, (min / 1000.0), (max / 1000.0),
         (sum / NUM_OF_TIMES) / 1000.0, sum / 1000.0);
}
#else

#define NUM_OF_TIMES 10
void bench_gen_address(addr_security_t security) {
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  long run_time = 0;
  long min = 0, max = 0, sum = 0;
  tryte_t addr[NUM_TRYTES_ADDRESS];

  for (int i = 0; i < NUM_OF_TIMES; i++) {
    uint32_t curr_t = micros();
    generate_address_trytes((tryte_t *)seed, i, security, addr);
    run_time = micros() - curr_t;
    max = (i == 0 || run_time > max) ? run_time : max;
    min = (i == 0 || run_time < min) ? run_time : min;
    sum += run_time;
  }
  printf("security %d:\t%.3f\t%.3f\t%.3f\t%.3f\n", security, (min / 1000.0), (max / 1000.0),
         (sum / NUM_OF_TIMES) / 1000.0, sum / 1000.0);
}

void bench_shimmer_addr(){
  uint8_t seed[SHIMMER_SEED_SIZE];
  uint8_t addr[SHIMMER_ADDR_SIZE];
  // char addr_str[SHIMMER_ADDR_STR_SIZE];
  long run_time = 0;
  long min = 0, max = 0, sum = 0;

  seed_from_string(seed, "3aPVf52nNUV27xvvTL6XA972p8jHdFcJac498EteGVyJ");

  for(int i = 0; i < NUM_OF_TIMES; i++){
    uint32_t curr_t = micros();
    get_address(addr, seed, i);
    run_time = micros() - curr_t;
    max = (i == 0 || run_time > max) ? run_time : max;
    min = (i == 0 || run_time < min) ? run_time : min;
    sum += run_time;
  }
  printf("shimmer:\t%.3f\t%.3f\t%.3f\t%.3f\n", (min / 1000.0), (max / 1000.0),
         (sum / NUM_OF_TIMES) / 1000.0, sum / 1000.0);
}

TaskHandle_t xHandle = NULL;
void task_bench_addr(void * pvParameters){
  printf("Bench address generation: %d times\n.\t\tmin(ms)\t\tmax(ms)\t\tavg(ms)\t\ttotal(ms)\n", NUM_OF_TIMES);
  bench_gen_address(ADDR_SECURITY_LOW);
  bench_gen_address(ADDR_SECURITY_MEDIUM);
  bench_gen_address(ADDR_SECURITY_HIGH);

  bench_shimmer_addr();
  vTaskSuspend(NULL);
}

#endif

void setup() {
  delay(2000);  // service delay
  UNITY_BEGIN();

  printf("ESP-IDF version %s\n", esp_get_idf_version());
  // RUN_TEST(test_is_trytes);
  // RUN_TEST(test_one_absorb);
  // RUN_TEST(test_gen_addresses);
#ifndef NEW_COMMON
  printf("Bench address generation: %d times\n\t\tmin(ms)\tmax(ms)\tavg(ms)\ttotal(ms)\n", NUM_OF_TIMES);
  bench_gen_address(1);
  bench_gen_address(2);
  bench_gen_address(3);
#else
  xTaskCreatePinnedToCore(task_bench_addr, "bench_addr", 81920, NULL, 1, &xHandle, 1);
  while(eTaskGetState(xHandle) != eSuspended){
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
#endif
  UNITY_END();  // stop unit testing
}

void loop() {}