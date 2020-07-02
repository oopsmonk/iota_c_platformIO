# IOTA Client library for PlatformIO IDE  

This project is tested on [Sipeed Maixduino](https://www.seeedstudio.com/Sipeed-Maixduino-Kit-for-RISC-V-AI-IoT-p-4047.html) 
[What is PlatformIO?](https://docs.platformio.org/en/latest/what-is-platformio.html)  
[Install PlatformIO in VSCode IDE](https://docs.platformio.org/en/latest/ide/vscode.html#platformio-ide-for-vscode)  

*Tested on*  
* PlatformIO Core: 4.3.4
* PlatformIO Home: 3.2.3
* framework-kendryte-freertos-sdk 0.7.0
* toolchain-kendryte210 8.2.0

## Getting Started  

Two ways to use IOTA common library in a [PlatformIO](https://platformio.org/) project. 

1. Clone this project

```
git clone https://github.com/oopsmonk/iota_c_platformIO.git 
cd iota_c_platformIO
git checkout origin/maixduino_freertos -b maixduino_freertos
```
You can use PlafromIO GUI to build and flash firmware or use command line interface: 

```bash
# Build firmware
cd iota_c_platformIO
pio run
# Flash firmware
kflash -p /dev/ttyUSB0 -b 1500000 -B maixduino ./.pio/build/maixduino_release/firmware.bin
```

2. Add dependencies libraries to your `platformio.ini`

```ini
[external_libs]
lib_deps_external =
    https://github.com/oopsmonk/iota_common.git#pio_lib
    https://github.com/troydhanson/uthash.git#1124f0a70b0714886402c3c0df03d037e3c4d57a
    https://github.com/oopsmonk/XKCP.git#pio_keccakp1600_inplace32

[env:maixduino]
platform = kendryte210
framework = kendryte-freertos-sdk
board = sipeed-maixduino
monitor_speed = 115200

build_flags =
     -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/common
     -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/low/common
     -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/high/Keccak
     -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/high/Keccak/FIPS202
     -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/low/KeccakP-1600/Inplace32

; Library options
lib_deps =
    ${external_libs.lib_deps_external}
```
