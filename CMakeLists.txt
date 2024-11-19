cmake_minimum_required(VERSION 3.10)
project(mydds)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Protobuf REQUIRED)
# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 获取 src 目录下所有的 .cpp 源文件
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.cc)



# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${PROJECT_SOURCE_DIR}/main.cpp)

# 设置编译输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 可选的：链接库（如果有的话）
target_link_libraries(${PROJECT_NAME} ${PROTOBUF_LIBRARIES})



#test
# set(GTest_DIR /usr/local/lib/cmake/GTest)
find_package(GTest REQUIRED)
# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://ghp.ci/https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
#   # DOWNLOAD_EXTRACT_TIMESTAMP TRUE
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

include_directories(${PROJECT_SOURCE_DIR}/data)
file(GLOB_RECURSE TESTSOURCES ${PROJECT_SOURCE_DIR}/test/*.cpp ${PROJECT_SOURCE_DIR}/test/*.cc ${PROJECT_SOURCE_DIR}/data/*.cc)
# 添加测试文件
enable_testing()

add_executable(
  hello_test
  ${TESTSOURCES}
  ${SOURCES}
)
target_link_libraries(
  hello_test
  GTest::gtest_main
  ${PROTOBUF_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(hello_test)