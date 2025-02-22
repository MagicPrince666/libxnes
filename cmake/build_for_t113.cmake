######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Linux)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "arm")

# 工具链地址
SET(TOOLCHAIN_DIR  "/home/leo/Tina-t113-pro/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/")

#export STAGING_DIR=/home/leo/Tina-t113-pro/out/t113-mq/staging_dir
# execute_process(COMMAND 
#     export STAGING_DIR=/home/leo/Tina-t113-pro/out/t113-mq/staging_dir
# )

# 设置头文件所在目录
include_directories(
    # ${TOOLCHAIN_DIR}../arm-openwrt-linux-muslgnueabi/include
    include/rv1126
)

# 设置第三方库所在位置
# link_directories(
#     ${TOOLCHAIN_DIR}../arm-openwrt-linux-muslgnueabi/lib
# )

# arm64 rockchip rk3308
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}arm-openwrt-linux-muslgnueabi-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}arm-openwrt-linux-muslgnueabi-g++)
