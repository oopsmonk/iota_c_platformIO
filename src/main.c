
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "common/crypto/kerl/kerl.h"
#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"
#include "utils/input_validators.h"

int64_t time_in_us() {
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  return (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
};

void test_gen_addresses(char *seed, size_t security) {
  int num_addrs = 3;
  char addr[NUM_TRYTES_ADDRESS + 1];
  addr[NUM_TRYTES_ADDRESS] = '\0';

  int64_t counter = 0;
  printf("Security = %ld\n", security);
  for (int i = 0; i < num_addrs; i++) {
    int64_t curr_t = time_in_us();
    // get addresses from security level 1.
    char *tmp_addr = iota_sign_address_gen_trytes(seed, i, security);
    counter = time_in_us() - curr_t;
    memcpy(addr, tmp_addr, NUM_TRYTES_ADDRESS);
    printf("[%d] %s\n", i, addr);
    printf("time spent: %ld us\n", counter);
    free(tmp_addr);
  }
}

int main() {
  sleep(5);
  puts("Hello World\n");
  test_gen_addresses("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 1);
  test_gen_addresses("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 2);
  test_gen_addresses("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 3);
  while (1) {
    printf("Hello\n");
    sleep(5);
  }
}
