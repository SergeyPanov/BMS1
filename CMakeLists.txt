cmake_minimum_required(VERSION 3.9)
project(BMS1)

set(CMAKE_CXX_STANDARD 11)

#add_subdirectory(libs/libsndfile)
find_package(SndFile REQUIRED)

add_executable(BMS1
    bms2A.cpp
    )
target_link_libraries(BMS1 PRIVATE SndFile::sndfile m)

add_executable(BMS2 bms2B.cpp)
target_link_libraries(BMS2 PRIVATE SndFile::sndfile m)