# IOTA Common library for PlatformIO IDE  

This project is tested on [ESP32-DevKitC V4](https://docs.espressif.com/projects/esp-idf/en/latest/hw-reference/get-started-devkitc.html#functional-description) 

[What is PlatformIO?](https://docs.platformio.org/en/latest/what-is-platformio.html)  

[Install PlatformIO in VSCode IDE](https://docs.platformio.org/en/latest/ide/vscode.html#platformio-ide-for-vscode)  

## Getting Started  

Two ways to use IOTA common library in a [PlatformIO](https://platformio.org/) project. 

1. Clone this project

```
git clone https://github.com/oopsmonk/iota_c_platformIO.git 
cd iota_c_platformIO
git checkout origin/arduino_esp32 -b arduino_esp32
```

Run tests  

```bash
cd iota_c_platformIO
pio test
```

2. Add dependencies libraries to your `platformio.ini`

```ini
[external_libs]
lib_deps_external =
    https://github.com/oopsmonk/iota_common.git#pio_lib
    https://github.com/troydhanson/uthash.git#1124f0a70b0714886402c3c0df03d037e3c4d57a
    https://github.com/oopsmonk/XKCP.git#pio_keccakp1600

[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

build_flags =
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/common
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/high/Keccak
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/high/Keccak/FIPS202
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/low/KeccakP-1600/Reference

; Library options
lib_deps =
    ${external_libs.lib_deps_external}
```
