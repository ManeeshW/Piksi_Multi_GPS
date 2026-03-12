cmake_minimum_required(VERSION 3.10)
project(PiksiMultiGPS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

find_package(Eigen3 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSERIALPORT REQUIRED libserialport)
include_directories(${LIBSERIALPORT_INCLUDE_DIRS})
link_directories(${LIBSERIALPORT_LIBRARY_DIRS})

# Add Zenoh dependencies (assuming zenoh-c is installed system-wide)
find_package(zenohc REQUIRED)
include_directories(${zenohc_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/libraries/libsbp/c/include)

add_executable(piksi_gps
    src/main.cpp
    src/piksi_multi_gps.cpp
)

target_link_libraries(piksi_gps
    sbp
    ${LIBSERIALPORT_LIBRARIES}
    zenohc::lib
    Eigen3::Eigen                
    nlohmann_json::nlohmann_json 
)
target_compile_options(piksi_gps
    PUBLIC -Wall -std=c++17
)

# Add this line to define ZENOHCXX_ZENOHC for Zenoh C++ bindings
target_compile_definitions(piksi_gps PUBLIC ZENOHCXX_ZENOHC)