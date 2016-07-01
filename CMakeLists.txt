project(TasksDeamon)
set(PROJECT_EXE "tasksd")
cmake_minimum_required(VERSION 2.8)

file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.cpp)

include_directories(. include/ utils/*)

find_library(MYSQLPP_LIB mysqlpp)
find_path(MYSQLPP_INCLUDE_PATH mysql++.h /usr/include/mysql++/)
find_path(MYSQL_INCLUDE_PATH mysql.h /usr/include/mysql/)

include_directories(${MYSQL_INCLUDE_PATH} ${MYSQLPP_INCLUDE_PATH})

add_executable(${PROJECT_EXE} ${SOURCES} ${HEADERS})

set(PROJECT_LIBS -lproxygenhttpserver -lfolly -lglog -lgflags -pthread ${MYSQLPP_LIB})

target_compile_options(${PROJECT_EXE} PUBLIC -std=c++14)
target_link_libraries(${PROJECT_EXE} PUBLIC ${PROJECT_LIBS})

