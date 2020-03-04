#include <Arduino.h>
#include <unity.h>

#include "common/crypto/kerl/kerl.h"
#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"
#include "test_iota_common.hpp"
#include "utils/input_validators.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

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

void setup() {
  delay(2000);  // service delay
  UNITY_BEGIN();

  printf("ESP-IDF version %s\n", esp_get_idf_version());
  RUN_TEST(test_is_trytes);
  RUN_TEST(test_one_absorb);
  RUN_TEST(test_gen_addresses);

  UNITY_END();  // stop unit testing
}

void loop() {}