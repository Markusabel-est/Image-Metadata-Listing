cmake_minimum_required(VERSION 3.30)
project(exif-finder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEXIF REQUIRED libexif)

add_executable(exif-finder main.cpp)

target_include_directories(exif-finder PRIVATE ${LIBEXIF_INCLUDE_DIRS})
target_link_directories(exif-finder PRIVATE ${LIBEXIF_LIBRARY_DIRS})
target_link_libraries(exif-finder PRIVATE ${LIBEXIF_LIBRARIES})