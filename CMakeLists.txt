cmake_minimum_required(VERSION 3.16)

project(MandelbrotSetParallelProgramming LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

find_package(OpenMP REQUIRED)

add_executable(mandel
    main.cpp
    mandel.cpp
)

target_link_libraries(mandel PRIVATE OpenMP::OpenMP_CXX)

find_package(MPI REQUIRED)

add_executable(mandel_mpi
    mandel_mpi.cpp
)

target_link_libraries(mandel_mpi PRIVATE MPI::MPI_CXX)