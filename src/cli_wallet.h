#pragma once

// #include "wallet/wallet.h"

#define WALLET_VERSION_MAJOR 0
#define WALLET_VERSION_MINOR 0
#define WALLET_VERSION_MICRO 1

#define VER_STR0(s) #s
#define VER_STR(s) VER_STR0(s)

#define APP_WALLET_VERSION      \
  VER_STR(WALLET_VERSION_MAJOR) \
  "." VER_STR(WALLET_VERSION_MINOR) "." VER_STR(WALLET_VERSION_MICRO)

// Register system commands
void register_wallet_commands();
int init_wallet();