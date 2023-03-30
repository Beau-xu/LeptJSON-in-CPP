cmake_minimum_required (VERSION 3.2)
project (leptjson_test CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(leptjson leptjson.cpp)              # 分别是库名（无后缀）、源文件名
add_executable(${PROJECT_NAME} test.cpp)          # 项目名、源文件
target_link_libraries(${PROJECT_NAME} leptjson)   # 给项目添加库
