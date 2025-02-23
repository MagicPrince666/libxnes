CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
option(EXAMPLE "shall we complie example" on)
# INCLUDE(build_for_rv1126)

project(xnes VERSION 0.1 LANGUAGES C CXX)

# set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_BUILD_TYPE "Debug")
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
add_definitions(-DBACKTRACE_DEBUG)
endif ()
set(CMAKE_CXX_STANDARD 11)

message(STATUS "System : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Target system processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Host architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")

# 执行git命令，并把结果重定向到自定义变量（此处是我的变量<GIT_VERSION>）。
execute_process(
	COMMAND	git log -1 --format=%H 		
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
	OUTPUT_VARIABLE GIT_VERSION
)
execute_process(
	COMMAND git remote -v 			
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
	OUTPUT_VARIABLE GIT_REMOTE_PATH
)

# 对得到的结果进行处理，尤其注意 \n,\t,\r之类的特殊字符，在cmake时没问题，但是生成的.cmake文件有问题，导致make出错。
string (REGEX REPLACE ".*\t| .*" "" GIT_PATH ${GIT_REMOTE_PATH})
string (REGEX REPLACE "[\n\t\r]" "" GIT_VERSION ${GIT_VERSION})

# 增加编译选项，把宏导入源码
add_definitions( -DGIT_VERSION=\"${GIT_VERSION}\")
add_definitions( -DGIT_PATH=\"${GIT_PATH}\")

add_definitions(-DPROJECT_PATH="${PROJECT_SOURCE_DIR}")

set(COMMON_LIBRARIES
    dl
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR})
  # 直接编译
  message(STATUS "host compile")
  find_package(SDL2 REQUIRED)
  list(APPEND COMMON_LIBRARIES ${SDL2_LIBRARIES})
else()
  # 交叉编译fb应用
  message(STATUS "cross compile")
endif()
  list(APPEND COMMON_LIBRARIES stdc++fs atomic)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
if (${MACHINES} MATCHES "arm64")
  message(STATUS "apple slicon compile")
else()
  message(STATUS "intel compile")
endif()
  find_package(SDL2 REQUIRED)
  list(APPEND COMMON_LIBRARIES ${SDL2_LIBRARIES})
  include_directories(${SDL2_INCLUDE_DIRS})
else()
  # windows not support
  message(STATUS "System : ${CMAKE_SYSTEM_NAME} not support")
endif()

include_directories(
	src
	examples/linux/framebuffer/
	${SDL2_INCLUDE_DIRS}
)

file(GLOB main_srcs 
    src/*.c
	src/mapper/*.c
	examples/linux/framebuffer/*.cpp
	examples/linux/main.cpp
)

add_executable(${PROJECT_NAME} ${main_srcs})

target_link_libraries(${PROJECT_NAME} 
	${COMMON_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)