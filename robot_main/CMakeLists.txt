cmake_minimum_required(VERSION 3.14)
project(RobotVoiceAssistant C CXX)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(TOOLCHAIN_PATH "$ENV{HOME}/robot/robot_main/third_party/duo-buildroot-sdk/host-tools/gcc/riscv64-linux-musl-x86_64")

set(CMAKE_C_COMPILER   "${TOOLCHAIN_PATH}/bin/riscv64-unknown-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/riscv64-unknown-linux-musl-g++")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

include_directories(include)
add_executable(robot_main 
    src/main.cpp
    src/AudioCapture.cpp
    src/AudioBuffer.cpp
    src/SpeechDetector.cpp
    src/client.cpp
)

target_link_libraries(robot_main PRIVATE
    asound
    pthread
)