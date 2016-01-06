project(TasksDeamon)
set(PROJECT_EXE "tasksd")
cmake_minimum_required(VERSION 3.2)

file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.cpp)

include_directories(. include/)

add_executable(${PROJECT_EXE} ${SOURCES} ${HEADERS})

target_compile_options(${PROJECT_EXE} PUBLIC -std=c++14)
target_link_libraries(${PROJECT_EXE} PUBLIC -lproxygenhttpserver -lfolly -lglog -lgflags -pthread)

