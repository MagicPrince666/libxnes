######## cross compile env define ###################
SET(CMAKE_SYSTEM_NAME Linux)
# 配置库的安装路径
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CMAKE_SYSTEM_PROCESSOR "aarch64")

# 工具链地址
SET(TOOLCHAIN_DIR  "/home/prince/rk3562/output/host/bin/")

# 设置头文件所在目录
include_directories(
    ${TOOLCHAIN_DIR}../aarch64-buildroot-linux-gnu/sysroot/usr/include
    include/rv1126
)

find_path(OpenCL_INCLUDE_DIR CL/cl.h PATHS ${TOOLCHAIN_DIR}../aarch64-buildroot-linux-gnu/sysroot/usr/include)

set(CMAKE_PREFIX_PATH ${TOOLCHAIN_DIR}/../aarch64-buildroot-linux-gnu/sysroot)

# 设置第三方库所在位置
link_directories(
    ${TOOLCHAIN_DIR}../aarch64-buildroot-linux-gnu/sysroot/usr/lib
)

# arm64 rockchip rk3308
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}aarch64-buildroot-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}aarch64-buildroot-linux-gnu-g++)
