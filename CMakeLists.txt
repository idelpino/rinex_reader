cmake_minimum_required(VERSION 2.8)
project(rinextk)

## To activate c++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

## Enable all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")


## Settings
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

SET(SRCS_RTK_LIB
	src/rinextk.cpp
)

## Build
add_executable(main src/main.cpp ${SRCS_RTK_LIB})

## Link
target_link_libraries(main gpstk)











