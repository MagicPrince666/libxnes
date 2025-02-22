######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Windows)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "x86_64")

# 工具链地址
SET(TOOLCHAIN_DIR  "D:\\msys2\\mingw64\\bin\\")

# 设置头文件所在目录
include_directories(
)

# 设置第三方库所在位置
link_directories(
)

SET(OPENSSL_CRYPTO_LIBRARY crypto)
SET(OPENSSL_SSL_LIBRARY ssl)

# windows mingw
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}gcc.exe)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}g++.exe)
