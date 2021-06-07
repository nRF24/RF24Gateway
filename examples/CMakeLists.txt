cmake_minimum_required(VERSION 3.12)

project(RF24GatewayExamples)
add_compile_options(-Ofast -Wall) # passing the compiler a `-pthread` flag doesn't work here

# detect the CPU make and type
include(../cmake/detectCPU.cmake) # sets the variable SOC accordingly

find_library(RF24 rf24 REQUIRED)
message(STATUS "using RF24 library: ${RF24}")

find_library(RF24Network rf24network REQUIRED)
message(STATUS "using RF24Network library: ${RF24Network}")

find_library(RF24Mesh rf24mesh REQUIRED)
message(STATUS "using RF24Mesh library: ${RF24Mesh}")

find_library(RF24Gateway rf24gateway REQUIRED)
message(STATUS "using RF24Gateway library: ${RF24Gateway}")

set(example RF24GatewayNode)
# make a target
add_executable(${example} ${example}.cpp)

# link the RF24 lib to the target. Notice we specify pthread as a linked lib here
target_link_libraries(${example} PUBLIC
    ${RF24}
    pthread
    ${RF24Network}
    ${RF24Mesh}
    ${RF24Gateway}
    )

add_subdirectory(ncurses)
add_subdirectory(ncursesInt)
add_subdirectory(gwNodeInt)
add_subdirectory(addons/Sniffer)