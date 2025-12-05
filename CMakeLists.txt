cmake_minimum_required(VERSION 3.16)
project(mini-db VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES src/*.cpp)
include_directories(include)
add_executable(mini-db ${SOURCES})