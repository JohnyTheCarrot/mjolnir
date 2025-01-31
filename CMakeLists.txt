cmake_minimum_required(VERSION 3.30)
project(mjolnir)

set(CMAKE_CXX_STANDARD 23)

add_library(mjolnir STATIC
        src/span.cpp
        include/mjolnir/span.hpp
        src/source.cpp
        include/mjolnir/source.hpp
        include/mjolnir/draw.hpp
        include/mjolnir/report.hpp
        src/report.cpp
        include/mjolnir/color.hpp
        src/color.cpp
)
target_include_directories(mjolnir PRIVATE include)

add_executable(example1
        example/example1.cpp
)
add_dependencies(example1 mjolnir)
target_link_libraries(example1 mjolnir)
target_include_directories(example1 PRIVATE include)
