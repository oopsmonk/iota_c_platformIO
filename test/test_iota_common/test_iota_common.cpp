#include "test_iota_common.hpp"

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
  Timer t;
  int num_addrs = 3;
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  printf("Test SEED: %s\n", seed);

  t.start();
  int counter = 0;
  printf("Security = 1\n");
  for (int i = 0; i < num_addrs; i++) {
    t.reset();
    // get addresses from security level 1.
    char *addr = iota_sign_address_gen_trytes(seed, i, 1);
    TEST_ASSERT_NOT_NULL(addr);
    counter = t.read_us();
    printf("[%d] %s\n", i, addr);
    printf("time spent: %d us\n", counter);
    TEST_ASSERT_EQUAL_MEMORY(expected_addr1[i], addr, NUM_TRYTES_HASH);
    free(addr);
  }

  printf("Security = 2\n");
  for (int i = 0; i < num_addrs; i++) {
    t.reset();
    // get addresses from security level 2.
    char *addr = iota_sign_address_gen_trytes(seed, i, 2);
    TEST_ASSERT_NOT_NULL(addr);
    counter = t.read_us();
    printf("[%d] %s\n", i, addr);
    printf("time spent: %d us\n", counter);
    TEST_ASSERT_EQUAL_MEMORY(expected_addr2[i], addr, NUM_TRYTES_HASH);
    free(addr);
  }

  printf("Security = 3\n");
  for (int i = 0; i < num_addrs; i++) {
    t.reset();
    // get addresses from security level 3.
    char *addr = iota_sign_address_gen_trytes(seed, i, 3);
    TEST_ASSERT_NOT_NULL(addr);
    counter = t.read_us();
    printf("[%d] %s\n", i, addr);
    printf("time spent: %d us\n", counter);
    TEST_ASSERT_EQUAL_MEMORY(expected_addr3[i], addr, NUM_TRYTES_HASH);
    free(addr);
  }
}

int main(int argc, char **argv) {
  thread_sleep_for(1000);
  UNITY_BEGIN();
  printf("Mbed OS version: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  RUN_TEST(test_is_trytes);
  RUN_TEST(test_one_absorb);
  RUN_TEST(test_gen_addresses);
  UNITY_END();

  return 0;
}
