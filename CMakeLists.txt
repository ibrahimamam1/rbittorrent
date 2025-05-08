cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0167 NEW)
project(bittorrent-starter-cpp)

# Enable C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-g -O0)

# Collect source files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
# Remove Main.cpp from SOURCE_FILES
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*Main\\.cpp$")

# Collect test files
file(GLOB_RECURSE TEST_FILES test/*.cpp)

# Find Boost, requiring at least version 1.74
find_package(Boost 1.74 REQUIRED COMPONENTS url system)

#Find Openssl
find_package(OpenSSL REQUIRED)

#find tbb. (needed for c++ parralel execution algorithms)
find_package(TBB REQUIRED)
find_package(Threads REQUIRED)

# Main executable
add_executable(rbittorrent src/Main.cpp ${SOURCE_FILES})
target_link_libraries(rbittorrent PRIVATE
    Boost::url
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    TBB::tbb
    Threads::Threads
)

# Google Test setup
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Test executable
add_executable(bencode_test ${TEST_FILES} ${SOURCE_FILES})

# Link against Google Test, pthread, and OpenSSL
target_link_libraries(bencode_test PRIVATE
    GTest::GTest
    GTest::Main
    pthread
    Boost::url
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    TBB::tbb
)

# Enable test discovery in CTest
enable_testing()
add_test(NAME BencodeTests COMMAND bencode_test)

