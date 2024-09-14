cmake_minimum_required(VERSION 3.16)

project(serverFrutibandas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT ${CMAKE_BINARY_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(enet REQUIRED)
include_directories(include)

set(SRCS src/main.cpp src/network_server.cpp src/game.cpp src/logic.cpp)

set(HEADERS include/network_server.hpp include/game.hpp include/logic.hpp include/random.hpp)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} Threads::Threads enet::enet)
