cmake_minimum_required(VERSION 3.16)

project(serverFrutibandas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT ${CMAKE_BINARY_DIR})

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	set(CONAN_DISABLE_CHECK_COMPILER TRUE)
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()
else()
	message(WARNING "The file conanbuildinfo.cmake doesn't exists, you have to run conan install first")
endif()

include_directories(include)

set(SRCS src/main.cpp src/network_server.cpp src/game.cpp src/logic.cpp)

set(HEADERS include/network_server.hpp include/game.hpp include/logic.hpp include/random.hpp)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	find_package(OPENMP RAQUIRED)
	if(OPENMP_FOUND)
		target_link_libraries(${PROJECT_NAME} ${OpenMP_LD_FLAGS})
	else()
		message(FATAL_ERROR "OpenMP not found.")
	endif()
endif()

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
