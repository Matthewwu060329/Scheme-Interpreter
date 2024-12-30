cmake_minimum_required(VERSION 3.16)
project(src)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(src
    Def.cpp
    Def.hpp
    evaluation.cpp
    expr.cpp
    expr.hpp
    main.cpp
    parser.cpp
    RE.cpp
    RE.hpp
    shared.hpp
    syntax.cpp
    syntax.hpp
    value.cpp
    value.hpp)
