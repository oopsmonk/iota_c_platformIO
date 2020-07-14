#include <stdio.h>
#include "mbed.h"

#define NEW_COMMON

#ifndef NEW_COMMON
#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"
#else
#include "model/address.h"
#include "model/transaction.h"
#endif

#define USE_RANDOM_SEED 0

tryte_t tryte_table[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
                           'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '9'};

int main() {
  Timer t;
  thread_sleep_for(1000);

  printf("Mbed OS version: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

#if USE_RANDOM_SEED
  // random seed
  char seed[NUM_TRYTES_HASH + 1] = {};
  srand(time(NULL));
  for (uint32_t idx = 0; idx < 81; idx++) {
    memset(seed + idx, tryte_table[rand() % 27], 1);
  }
  seed[NUM_TRYTES_HASH] = '\0';

  printf("RANDOM SEED: %s\n", seed);
#else
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  printf("SEED: %s\n", seed);
#endif

#ifndef NEW_COMMON
  t.start();
  int counter = 0;
  for (int i = 0; i < 5; i++) {
    t.reset();
    // get addresses from security level 2.
    char *addr = iota_sign_address_gen_trytes(seed, i, 2);
    counter = t.read_us();
    printf("[%d] %s\n", i, addr);
    printf("time spent: %d us\n", counter);
    free(addr);
  }
#else
  t.start();
  int counter = 0;
  tryte_t addr[NUM_TRYTES_ADDRESS];
  for (int i = 0; i < 5; i++) {
    t.reset();
    // get addresses from security level 2.
    generate_address_trytes((tryte_t *)seed, i, ADDR_SECURITY_MEDIUM, addr);
    counter = t.read_us();
    printf("[%d] %.*s\n", i, NUM_TRYTES_ADDRESS, addr);
    printf("time spent: %d us\n", counter);
  }

#endif
  return 0;
}
