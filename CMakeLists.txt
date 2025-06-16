cmake_minimum_required(VERSION 3.25)
project(logger CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
	main.cpp logger.hpp metrics.hpp
)
