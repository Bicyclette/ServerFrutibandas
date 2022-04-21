cmake_minimum_required(VERSION 3.16)

project(serverFrutibandas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT ${CMAKE_BINARY_DIR})

include_directories(include ${ENET_INCLUDE_DIR})

link_directories(${ENET_LIB_DIR})

set(SRCS src/main.cpp src/network_server.cpp)

set(HEADERS include/network_server.hpp)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

target_link_libraries(${PROJECT_NAME} ${ENET_LIBS})