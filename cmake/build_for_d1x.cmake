######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Linux)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "riscv64")

# 工具链地址
SET(TOOLCHAIN_DIR  "/home/leo/tina/out/d1s-mq/staging_dir/toolchain/bin/")

# 设置头文件所在目录
include_directories(
    ${TOOLCHAIN_DIR}../../target/usr/include
    ${TOOLCHAIN_DIR}../../target/usr/include/dbus-1.0
    ${TOOLCHAIN_DIR}../../target/usr/include/glib-2.0
    ${TOOLCHAIN_DIR}../../target/usr/lib/dbus-1.0/include
    ${TOOLCHAIN_DIR}../../target/usr/lib/glib-2.0/include
)

# 设置第三方库所在位置
link_directories(
    ${TOOLCHAIN_DIR}../../target/usr/lib
)
SET(OPENSSL_CRYPTO_LIBRARY crypto)
SET(OPENSSL_SSL_LIBRARY ssl)

# sunxi t113
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}riscv64-unknown-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}riscv64-unknown-linux-gnu-g++)
