#!/bin/bash
source .venv/bin/activate
mkdir -p build
cd build
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=/home/simon/Downloads/gcc-arm-none-eabi -DNRF5_SDK_PATH=/home/simon/source/InfiniTime/nrf5_sdk -DBUILD_DFU=1 ..
cd ..
make -j pinetime-mcuboot-app
