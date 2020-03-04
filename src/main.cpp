#include <Arduino.h>

#include "common/helpers/sign.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello ESP32 Arduino!");
  char const *seed = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  printf("SEED: %s\n", seed);
  char *addr = iota_sign_address_gen_trytes(seed, 0, 2);
  printf("%s\n", addr);
  free(addr);
  delay(10000);
}