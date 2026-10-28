cmake_minimum_required(VERSION 3.16)

project(scientific_calculator
    VERSION 1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(scientific_calculator
    main.c
    calculator.c
    parser.c
    command.c
)

target_compile_options(scientific_calculator PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Wconversion -Wshadow>
)

if(NOT WIN32)
    target_link_libraries(scientific_calculator PRIVATE m)
endif()
