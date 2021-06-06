cmake_minimum_required(VERSION 3.0)
project(mkStatemachine CXX)
include(cmake/StandardProjectSettings.cmake)

#if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
#message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
#file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
#                "${CMAKE_BINARY_DIR}/conan.cmake")
#endif()

#include(${CMAKE_BINARY_DIR}/conan.cmake)
#conan_cmake_run(CONANFILE conanfile.txt
#                BASIC_SETUP CMAKE_TARGETS
#                BUILD missing)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_library(standard_options INTERFACE)
target_link_libraries(standard_options INTERFACE project_options)
target_link_libraries(standard_options INTERFACE project_warnings)

add_executable(statemachine main.cpp)
target_link_libraries(statemachine PRIVATE standard_options)

add_executable(sub substatemachine.cpp)
target_link_libraries(sub PRIVATE standard_options)

