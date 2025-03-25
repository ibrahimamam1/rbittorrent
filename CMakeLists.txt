cmake_minimum_required(VERSION 3.13)

project(bittorrent-starter-cpp)

# Enable C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Collect source and test files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
# Remove Main.cpp from SOURCE_FILES
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*Main\\.cpp$")

file(GLOB_RECURSE TEST_FILES test/*.cpp)

# Main executable
add_executable(bencode src/Main.cpp ${SOURCE_FILES})

# Google Test setup
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Test executable
add_executable(bencode_test ${TEST_FILES} ${SOURCE_FILES})

# Link against Google Test and pthread
target_link_libraries(bencode_test PRIVATE GTest::GTest GTest::Main pthread)

# Enable test discovery in CTest
enable_testing()
add_test(NAME BencodeTests COMMAND bencode_test)

