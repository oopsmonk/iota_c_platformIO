# IOTA Chrysalis CLI Wallet

A simple CLI wallet to interact with IOTA Chrysalis network on ESP32
A CLI wallet to interact with Pollen network on ESP32.

# System requirements

* [VSCode with PlaftormIO](https://platformio.org/install/ide?install=vscode)  
* [espressif32](https://docs.platformio.org/en/latest/platforms/espressif32.html)  

# Support Commands (WIP)

* `balance`: Show balance from address index
* `address`: Get address hash from address index
* `send`: send transaction

# Disclaimer

This application is based on [iota.c](https://github.com/iotaledger/iota.c/tree/30b678e8957cd98393ab6265880e2f453be6a69f), it is under developing and tested with [gohornet/hornet](https://github.com/gohornet/hornet/tree/1de8b74683c2a4143531c31f0e34d0dbcb534443) and [espressif32 v2.1.0](https://github.com/platformio/platform-espressif32/releases/tag/v2.1.0).  

It's not guarantee to work on other release or system, please use with caution!  

# Building and Running

## Checkout the iota_c_platformIO project and opening it on VSCode IDE

```
git clone https://github.com/oopsmonk/iota_c_platformIO.git
cd iota_c_platformIO
git checkout origin/dev_pollen_esp32 -b dev_esp32_chrysalis
code .
```

## Project Configuration  

Find the `Run Menuconfig` in PlatformIO extension  

**Setup endpoint and wallet information is needed**  

![Image](http://i.imgur.com/Mq1logq.png)

## Upload and Monitor 

Run the application on ESP32.

![Image](http://i.imgur.com/uohncEf.png)
