cmake_minimum_required(VERSION 3.15)
project(companion_device CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# == lama.cpp == #
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/llama.cpp)

# == whisper.cpp == #
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/whisper.cpp)

# == kokoro.cpp == #
file(GLOB PROJECT_SOURCES "src/*.cpp")
set(KOKORO_SOURCES
    third_party/kokoro.cpp/Tokenizer.cpp
    third_party/kokoro.cpp/PinyinFinder.cpp
    third_party/kokoro.cpp/ToneSandhi.cpp
    third_party/kokoro.cpp/ZHFrontend.cpp
    third_party/kokoro.cpp/ZHG2P.cpp
)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${KOKORO_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    third_party/llama.cpp/include
    third_party/kokoro.cpp
    third_party/kokoro.cpp/third_party/cppjieba/include
    third_party/kokoro.cpp/third_party/cppjieba/deps/limonp/include
    third_party/onnxruntime-linux/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime-linux/lib
)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    whisper
    llama
    asound
    pthread
    Threads::Threads
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime-linux/lib/libonnxruntime.so
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime-linux/lib"
)