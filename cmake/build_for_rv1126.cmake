######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Linux)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "arm")

# 工具链地址
SET(TOOLCHAIN_DIR  "/home/prince/rv1126_ubuntu_sdk/buildroot/output/firefly_rv1126_rv1109/host/bin/")

# 设置头文件所在目录
include_directories(
    ${TOOLCHAIN_DIR}../arm-buildroot-linux-gnueabihf/sysroot/usr/include
)

set(CMAKE_PREFIX_PATH ${TOOLCHAIN_DIR}/../arm-buildroot-linux-gnueabihf/sysroot/usr)

# 设置第三方库所在位置
link_directories(
    ${TOOLCHAIN_DIR}../arm-buildroot-linux-gnueabihf/sysroot/usr/lib
)

# rockchip rv1126
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}arm-buildroot-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}arm-buildroot-linux-gnueabihf-g++)
