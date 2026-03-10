cmake_minimum_required(VERSION 4.2.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_MODULE_STD 1)
project(RandGame VERSION 0.1.0 LANGUAGES CXX)
ADD_DEFINITIONS (-Wall -Wextra -pedantic -Wno-deprecated-declarations -g)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
