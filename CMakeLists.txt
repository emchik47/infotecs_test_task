cmake_minimum_required(VERSION 3.10)
project(Logger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(logger_lib SHARED lib/logger.cpp lib/logger.h)
target_include_directories(logger_lib PUBLIC lib)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE logger_lib Threads::Threads)

# add_executable(stat_server server.cpp)
