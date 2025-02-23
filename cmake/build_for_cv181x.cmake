######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Linux)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "riscv64")

# 工具链地址
SET(TOOLCHAIN_DIR  "/home/prince/duo-buildroot-sdk/buildroot-2021.05/output/milkv-duos-emmc_musl_riscv64/host/bin/")

# 设置头文件所在目录
include_directories(
    ${TOOLCHAIN_DIR}../riscv64-buildroot-linux-musl/sysroot/usr/include
    include/rv1126
)

# 设置第三方库所在位置
link_directories(
    ${TOOLCHAIN_DIR}../riscv64-buildroot-linux-musl/sysroot/usr/lib
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")

# sunxi t113
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}riscv64-unknown-linux-musl-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}riscv64-unknown-linux-musl-g++)
