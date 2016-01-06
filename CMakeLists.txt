project(TasksDeamon)
set(PROJECT_EXE "tasksd")
cmake_minimum_required(VERSION 3.2)
aux_source_directory(src/ SRC_LIST)
include_directories(. include/)

add_executable(${PROJECT_EXE} ${SRC_LIST})

target_compile_options(${PROJECT_EXE} PUBLIC -std=c++14)
target_link_libraries(${PROJECT_EXE} PUBLIC -lproxygenhttpserver -lfolly -lglog -lgflags -pthread)

