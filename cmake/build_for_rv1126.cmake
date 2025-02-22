######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Linux)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "arm")

# 工具链地址
# SET(TOOLCHAIN_DIR  "/home/leo/rv1126_ubuntu_sdk/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/")
SET(TOOLCHAIN_DIR  "/home/leo/rv1126_ubuntu_sdk/buildroot/output/dreame_alientek_rv1126_rv1109/host/bin/")

# 设置头文件所在目录
include_directories(
    ${TOOLCHAIN_DIR}../arm-buildroot-linux-gnueabihf/sysroot/usr/include
    include/rv1126
)

set(LIBUSB_1_INCLUDE_DIRS ${TOOLCHAIN_DIR}/../arm-buildroot-linux-gnueabihf/sysroot/usr/include/libusb-1.0)

# 设置第三方库所在位置
link_directories(
    ${TOOLCHAIN_DIR}../arm-buildroot-linux-gnueabihf/sysroot/usr/lib
)

SET(OPENSSL_CRYPTO_LIBRARY ${TOOLCHAIN_DIR}../arm-buildroot-linux-gnueabihf/sysroot/usr/lib/libcrypto.so)
SET(OPENSSL_SSL_LIBRARY ${TOOLCHAIN_DIR}../arm-buildroot-linux-gnueabihf/sysroot/usr/lib/libssl.so)

# arm64 rockchip rk3308
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}arm-linux-gnueabihf-g++)
