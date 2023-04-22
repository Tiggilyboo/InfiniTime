#!/bin/bash
source .venv/bin/activate
mkdir -p build
cd build
cmake \
  -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-10.3-2021.10/ \
  -DBUILD_RESOURCES=0 \
  -DNRF5_SDK_PATH=/opt/nrf5-sdk/ -DBUILD_DFU=1 ..
make -j pinetime-mcuboot-app
