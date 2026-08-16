# RK3588 Buildroot ARM64 交叉编译工具链

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(RK3588_HOST
    /home/aaa/rk3588_linux_sdk/buildroot/output/rockchip_atk_dlrk3588/host
)

set(RK3588_SYSROOT
    ${RK3588_HOST}/aarch64-buildroot-linux-gnu/sysroot
)

set(CMAKE_SYSROOT ${RK3588_SYSROOT})

set(CMAKE_C_COMPILER
    ${RK3588_HOST}/bin/aarch64-buildroot-linux-gnu-gcc
)

set(CMAKE_CXX_COMPILER
    ${RK3588_HOST}/bin/aarch64-buildroot-linux-gnu-g++
)

set(CMAKE_FIND_ROOT_PATH
    ${RK3588_SYSROOT}
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)