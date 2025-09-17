cmake_minimum_required(VERSION 4.0)
project(pms)

# --- Language standard and portability
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pms
        main.cpp
        types.cpp
        instrument.cpp
        marketdata.cpp
        position.cpp)
