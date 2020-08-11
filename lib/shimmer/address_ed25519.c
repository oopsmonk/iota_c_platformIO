#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "address_ed25519.h"
#include "libbase58.h"
#include "sodium.h"

void dump_hex(uint8_t data[], size_t len) {
  for (int i = 0; i < len; i++) {
    printf("0x%x, ", data[i]);
  }
  printf("\n");
}

/**
 * @brief hexadecimal text to a string, ex: "48656c6c6f" -> "Hello"
 *
 * @param str the hex text,
 * @param array output string
 */
void hex_decode_string(const char str[], uint8_t array[]) {
  size_t len = strlen(str) / 2;
  for (size_t i = 0; i < len; i++) {
    uint8_t c = 0;
    if (str[i * 2] >= '0' && str[i * 2] <= '9') {
      c += (str[i * 2] - '0') << 4;
    }
    if ((str[i * 2] & ~0x20) >= 'A' && (str[i * 2] & ~0x20) <= 'F') {
      c += (10 + (str[i * 2] & ~0x20) - 'A') << 4;
    }
    if (str[i * 2 + 1] >= '0' && str[i * 2 + 1] <= '9') {
      c += (str[i * 2 + 1] - '0');
    }
    if ((str[i * 2 + 1] & ~0x20) >= 'A' && (str[i * 2 + 1] & ~0x20) <= 'F') {
      c += (10 + (str[i * 2 + 1] & ~0x20) - 'A');
    }
    array[i] = c;
  }
}

void random_seed(uint8_t seed[]) { randombytes_buf((void *const)seed, SHIMMER_SEED_SIZE); }

bool seed_2_string(char str_buf[], size_t *buf_len, uint8_t seed[]) {
  return b58enc((char *)str_buf, buf_len, (const void *)seed, SHIMMER_SEED_SIZE);
}

bool seed_from_string(uint8_t out_seed[], const char *str) {
  size_t out_len = SHIMMER_SEED_SIZE;
  return b58tobin((void *)out_seed, &out_len, str, strlen(str));
}

// subSeed generates the n'th sub seed of this Seed which is then used to generate the KeyPair.
static void get_subseed(uint8_t subseed[], uint8_t seed[], uint64_t index) {
  // convert index to 8-byte-array in little-endian
  uint8_t bytes_index[8];
  // TODO: hardware optimization
  bytes_index[0] = index >> 8 * 0;
  bytes_index[1] = index >> 8 * 1;
  bytes_index[2] = index >> 8 * 2;
  bytes_index[3] = index >> 8 * 3;
  bytes_index[4] = index >> 8 * 4;
  bytes_index[5] = index >> 8 * 5;
  bytes_index[6] = index >> 8 * 6;
  bytes_index[7] = index >> 8 * 7;
  // printf("Index: ");
  // dump_hex(bytes_index, 8);

  // hash index-byte
  uint8_t hash_index[32];
  crypto_generichash(hash_index, 32, bytes_index, 8, NULL, 0);
  // printf("hashIndex: ");
  // dump_hex(hash_index, 32);

  // XOR subseed and hashedIndexBytes
  memcpy(subseed, seed, SHIMMER_SEED_SIZE);
  // TODO: hardware optimization
  for (int i = 0; i < SHIMMER_SEED_SIZE; i++) {
    subseed[i] = subseed[i] ^ hash_index[i];
  }
  // printf("subseed: ");
  // dump_hex(subseed, SHIMMER_SEED_SIZE);
}

void get_address(uint8_t addr_out[], uint8_t seed[], uint64_t index) {
  // public key of the seed
  uint8_t pub_key[SHIMMER_PUBLIC_KEY_SIZE];
  uint8_t priv_key[SHIMMER_PRIVATE_KEY_SIZE];
  uint8_t subseed[SHIMMER_SEED_SIZE];
  // get subseed from seed
  get_subseed(subseed, seed, index);
  // get ed25519 public and private key from subseed
  crypto_sign_seed_keypair(pub_key, priv_key, subseed);
  // printf("pub: ");
  // dump_hex(pub, SHIMMER_PUBLIC_KEY_SIZE);
  // printf("private: ");
  // dump_hex(priv, SHIMMER_PRIVATE_KEY_SIZE);

  // digest: blake2b the public key
  uint8_t digest[SHIMMER_DIGEST_SIZE];
  crypto_generichash(digest, SHIMMER_DIGEST_SIZE, pub_key, SHIMMER_PUBLIC_KEY_SIZE, NULL, 0);
  // printf("digest: ");
  // dump_hex(digest, SHIMMER_DIGEST_SIZE);

  // address[0] = version, address[1:] = digest
  addr_out[0] = SHIMMER_ED25519_VERSION;
  memcpy((void *)(addr_out + 1), digest, 32);
}

bool address_2_string(char str_buf[], uint8_t address[]) {
  size_t buf_len = SHIMMER_ADDR_STR_SIZE;
  bool ret = b58enc(str_buf, &buf_len, (const void *)address, SHIMMER_ADDR_SIZE);
  printf("addr len %d\n", buf_len);
  return ret;
}

// signs the message with privateKey and returns a signature.
void sign_signature(uint8_t signature[], uint8_t seed[], uint64_t index, uint8_t data[], uint64_t data_len) {
  //
  uint8_t pub_key[SHIMMER_PUBLIC_KEY_SIZE];
  uint8_t priv_key[SHIMMER_PRIVATE_KEY_SIZE];
  uint8_t subseed[SHIMMER_SEED_SIZE];
  uint64_t sign_len = 0;
  // get subseed from seed
  get_subseed(subseed, seed, index);
  // get ed25519 public and private key from subseed
  crypto_sign_seed_keypair(pub_key, priv_key, subseed);

  crypto_sign(signature, &sign_len, data, data_len, priv_key);
  printf("sig len %"PRIu64"\n", sign_len);
}

bool sign_verify_signature(uint8_t seed[], uint64_t index, uint8_t signature[], uint8_t data[], size_t data_len) {
  uint8_t pub_key[SHIMMER_PUBLIC_KEY_SIZE];
  uint8_t priv_key[SHIMMER_PRIVATE_KEY_SIZE];
  uint8_t subseed[SHIMMER_SEED_SIZE];
  uint8_t exp_data[200];
  uint64_t exp_data_len = 0;
  // get subseed from seed
  get_subseed(subseed, seed, index);
  // get ed25519 public and private key from subseed
  crypto_sign_seed_keypair(pub_key, priv_key, subseed);
  if (crypto_sign_open(exp_data, &exp_data_len, signature, SHIMMER_SIGNATURE_SIZE + data_len, pub_key) == 0) {
    printf("data size %"PRIu64"\n", exp_data_len);
    return memcmp(data, exp_data, exp_data_len) ? false : true;
  } else {
    printf("failed\n");
    return false;
  }
}

void test_seed() {
  uint8_t seed[SHIMMER_SEED_SIZE];
  uint8_t addr[SHIMMER_ADDR_SIZE] = {0};
  char addr_str[SHIMMER_ADDR_STR_SIZE];

  seed_from_string(seed, "3aPVf52nNUV27xvvTL6XA972p8jHdFcJac498EteGVyJ");
  get_address(addr, seed, 0);
  // printf("addr 0: ");
  // dump_hex(addr, SHIMMER_ADDR_SIZE);
  // address_2_string(addr_str, addr);
  // printf("addr str: %s\n", addr_str);

  for (int i = 0; i < 10; i++) {
    get_address(addr, seed, i);
    address_2_string(addr_str, addr);
    printf("addr[%d]: %s\n", i, addr_str);
  }
}
