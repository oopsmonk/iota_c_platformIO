#pragma once

#include <stdbool.h>

#include "sodium.h"

// the length of an address (digest length = 32 + version byte length = 1).
#define SHIMMER_ADDR_SIZE 33
#define SHIMMER_ADDR_STR_SIZE 45
#define SHIMMER_SEED_SIZE crypto_sign_ed25519_SEEDBYTES
#define SHIMMER_PUBLIC_KEY_SIZE crypto_sign_ed25519_PUBLICKEYBYTES
#define SHIMMER_PRIVATE_KEY_SIZE crypto_sign_ed25519_SECRETKEYBYTES
#define SHIMMER_SIGNATURE_SIZE crypto_sign_ed25519_BYTES
#define SHIMMER_DIGEST_SIZE 32
#define SHIMMER_ED25519_VERSION 1


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets a random seed.
 * 
 * @param[out] seed An output seed
 */
void random_seed(uint8_t seed[]);

/**
 * @brief Gets a human readable version of the Seed (base58 encoded).
 * 
 * @param[out] str_buf The seed string
 * @param[in, out] buf_len The len of string
 * @param[in] seed The seed in bytes
 * @return true 
 * @return false 
 */
bool seed_2_string(char str_buf[], size_t *buf_len, uint8_t seed[]);

/**
 * @brief Gets seed bytes from a human readable seed string.
 * 
 * @param[out] out_seed seed in bytes array
 * @param[in] str the seed string
 * @return true 
 * @return false 
 */
bool seed_from_string(uint8_t out_seed[], const char *str);

/**
 * @brief Gets the address from corresponding seed and index
 * 
 * @param[out] addr_out An address
 * @param[in] seed The seed for genrate address
 * @param[in] index The index of address
 */
void get_address(uint8_t addr_out[], uint8_t seed[], uint64_t index);


/**
 * @brief Gets a human readable version of the address (base58 encoded).
 * 
 * @param[out] str_buf A buffer holds string address
 * @param[in] address An address in bytes
 * @return true 
 * @return false 
 */
bool address_2_string(char str_buf[], uint8_t address[]);

void sign_signature(uint8_t signature[], uint8_t seed[], uint64_t index, uint8_t data[], uint64_t data_len);

bool sign_verify_signature(uint8_t seed[], uint64_t index, uint8_t signature[], uint8_t data[], size_t data_len);

void dump_hex(uint8_t data[], size_t len);
void test_seed();

#ifdef __cplusplus
}
#endif
