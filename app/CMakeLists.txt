cmake_minimum_required(VERSION 3.18.1)
project("gpuvirtual")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mfpu=neon -march=armv8-a+simd")

add_library(gpuvirtual SHARED
    src/main/cpp/native-lib.cpp
    src/main/cpp/rasterizer.cpp
    src/main/cpp/math_utils.cpp
)

find_library(log-lib log)
target_link_libraries(gpuvirtual
    ${log-lib}
    android
    EGL
    GLESv2
)
