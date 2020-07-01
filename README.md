# IOTA Common library for PlatformIO IDE  

This project is tested on [NUCLEO-F746ZG](https://www.st.com/en/evaluation-tools/nucleo-f746zg.html) 

[What is PlatformIO?](https://docs.platformio.org/en/latest/what-is-platformio.html)  

[Install PlatformIO in VSCode IDE](https://docs.platformio.org/en/latest/ide/vscode.html#platformio-ide-for-vscode)  

*Tested On*  
* ST STM32 7.0.0 > ST Nucleo F746ZG 
* framework-mbed 6.51401.200622 (5.14.1) 
* toolchain-gccarmnoneeabi 1.70201.0 (7.2.1) 

## Getting Started  

Two ways to use IOTA common library in a [PlatformIO](https://platformio.org/) project. 

1. Clone this project

```
git clone https://github.com/oopsmonk/iota_c_platformIO.git 
cd iota_c_platformIO
git checkout origin/mbed_stm32f746zg -b mbed_stm32f746zg 
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
    https://github.com/oopsmonk/XKCP.git#pio_keccakp1600_inplace32

[env:nucleo_f746zg_release]
build_type = release
platform = ststm32
board = nucleo_f746zg
upload_protocol = stlink
framework = mbed

build_flags = 
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/common
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/low/common
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/high/Keccak
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/high/Keccak/FIPS202
    -I${PROJECT_LIBDEPS_DIR}/${PIOENV}/Keccak/lib/low/KeccakP-1600/Inplace32BI

; Library options
lib_deps =
    ${external_libs.lib_deps_external}
```
