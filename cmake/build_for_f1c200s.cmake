######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Linux)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "arm")

# 工具链地址
SET(TOOLCHAIN_DIR  "/home/prince/xos/output/host/bin/")

# 设置头文件所在目录
include_directories(
    ${TOOLCHAIN_DIR}../arm-f1c100s-linux-gnueabi/sysroot/usr/include
    include/rv1126
)

set(CMAKE_PREFIX_PATH ${TOOLCHAIN_DIR}/../arm-f1c100s-linux-gnueabi/sysroot/usr)

# 设置第三方库所在位置
link_directories(
    ${TOOLCHAIN_DIR}../arm-f1c100s-linux-gnueabi/sysroot/usr/lib
)

# arm64 rockchip rk3308
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}arm-f1c100s-linux-gnueabi-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}arm-f1c100s-linux-gnueabi-g++)
