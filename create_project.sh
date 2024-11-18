#!/bin/bash

# 检查是否提供了项目名
if [ -z "$1" ]; then
    echo "请提供一个项目名！"
    exit 1
fi

PROJECT_NAME=$1
SRC_DIR="src"
BUILD_DIR="build"
INCLUDE_DIR="include"
LIB_DIR="lib"
CMAKE_FILE="CMakeLists.txt"

# 创建项目目录结构
mkdir -p $PROJECT_NAME/$SRC_DIR
mkdir -p $PROJECT_NAME/$INCLUDE_DIR
mkdir -p $PROJECT_NAME/$LIB_DIR
mkdir -p $PROJECT_NAME/$BUILD_DIR

# 进入项目根目录
cd $PROJECT_NAME

# 创建简单的 C++ 源文件
cat <<EOL >$SRC_DIR/main.cpp
#include <iostream>

int main() {
    std::cout << "Hello, $PROJECT_NAME!" << std::endl;
    return 0;
}
EOL

# 创建一个简单的 CMakeLists.txt
cat <<EOL >$CMAKE_FILE
cmake_minimum_required(VERSION 3.10)
project($PROJECT_NAME)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 包含头文件目录
include_directories(\${PROJECT_SOURCE_DIR}/$INCLUDE_DIR)

# 设置源文件
set(SOURCE_FILES $SRC_DIR/main.cpp)

# 添加可执行文件
add_executable(\${PROJECT_NAME} \${SOURCE_FILES})

# 设置编译输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \${PROJECT_SOURCE_DIR}/bin)

# 可选的：链接库（如果有的话）
# target_link_libraries(\${PROJECT_NAME} \${LIB_DIR}/mylib)
EOL

# 提示信息
echo "C++ 项目 '$PROJECT_NAME' 已创建！"
echo "使用以下命令构建项目："
echo "cd $PROJECT_NAME"
echo "mkdir build && cd build"
echo "cmake .."
echo "make"
