# CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8)

# 项目信息
project("CodeCraft-2021")

# include路径 (.h路径)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/lib)

# 查找当前目录下的所有源文件 (.cpp路径)
aux_source_directory(. DIR_SRCS)
set(AllSrcFile ${DIR_SRCS})

# 设置可执行文件生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)

# 生成debug版本
#SET(CMAKE_BUILD_TYPE "debug")
# 生成release版本
SET(CMAKE_BUILD_TYPE "release")
if (CMAKE_BUILD_TYPE STREQUAL debug)
    add_definitions(-D_DEBUG)
endif ()

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -std=c++11")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -std=c++11")

# 指定生成目标
add_executable(CodeCraft-2021 ${AllSrcFile})

# 多线程
find_package(OpenMP REQUIRED)
target_link_libraries(CodeCraft-2021 PUBLIC OpenMP::OpenMP_CXX)