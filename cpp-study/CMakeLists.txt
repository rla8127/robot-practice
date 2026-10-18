cmake_minimum_required(VERSION 3.16)
project(cpp_study LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 경고를 최대한 켜 둔다. 경고는 "잠재적 버그"라고 생각하자.
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)

# 01은 파일이 여러 개라서 따로 등록한다.
add_executable(01_build_basics
    01_build_basics/main.cpp
    01_build_basics/motor.cpp
)

# 나머지는 폴더마다 main.cpp 하나씩이다.
set(SINGLE_FILE_EXAMPLES
    02_references_pointers
    03_object_lifetime
    04_raii
    05_smart_pointers
    06_copy_move
    07_stl_containers
    08_const_optional_errors
    09_polymorphism
    10_threads_mutex
    11_atomic_condvar
)

foreach(name IN LISTS SINGLE_FILE_EXAMPLES)
    add_executable(${name} ${name}/main.cpp)
    target_link_libraries(${name} PRIVATE Threads::Threads)
endforeach()
