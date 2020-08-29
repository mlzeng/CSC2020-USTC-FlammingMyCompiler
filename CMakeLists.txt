cmake_minimum_required(VERSION 3.16)
SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
project(SYSYCompiler)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# Import LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
llvm_map_components_to_libnames(
        llvm_libs
        all
)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


# Import LLVM

INCLUDE_DIRECTORIES(include/front)

include_directories(include/syntax_tree)

include_directories(include)
include_directories(include/IR)
include_directories(include/IRBuilder)
include_directories(include/CodeGen)
include_directories(include/OptimizePass)
#include_directories(include/llvm-builder)
include_directories(include/HIR)

add_subdirectory(src/front)

add_subdirectory(src/syntax_tree)
add_subdirectory(src/IRBuilder)
add_subdirectory(src/IR)
add_subdirectory(src/CodeGen)
add_subdirectory(src/OptimizePass)
#add_subdirectory(src/llvm-builder)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        compiler
        src/main.cc
)

target_link_libraries(
        compiler
        IRBuilder
        driver
        IR_lib
        Pass
        CodeGen
#        ControlTree
        ActiveVars
        RegisterAllocation
)

install(
        TARGETS compiler
        RUNTIME DESTINATION bin
)

