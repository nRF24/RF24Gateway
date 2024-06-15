cmake_minimum_required(VERSION 3.15)

# Set the project name to your project name
project(RF24Gateway C CXX)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
add_compile_options(-Ofast -Wall)

# allow using CMake options to adjust RF24Network_config.h without modiying source code
option(RF24GATEWAY_DEBUG_LEVEL "adjust the verbosity of the debugging messages" 0)

# detect CPU and add compiler flags accordingly
include(cmake/detectCPU.cmake)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if(ENABLE_BUILD_WITH_TIME_TRACE)
        add_compile_definitions(project_options INTERFACE -ftime-trace)
    endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# get library info from Arduino IDE's required library.properties file
include(cmake/GetLibInfo.cmake) # sets the variable LibTargetName

# setup CPack options
include(cmake/CPackInfo.cmake)

# detect any applicable external libs (like pigpio)
include(cmake/AutoConfig_RF24_DRIVER.cmake)

find_library(RF24 rf24 REQUIRED)
message(STATUS "using RF24 library: ${RF24}")

find_library(RF24Network rf24network REQUIRED)
message(STATUS "using RF24Network library: ${RF24Network}")

find_library(RF24Mesh rf24mesh REQUIRED)
message(STATUS "using RF24Mesh library: ${RF24Mesh}")

###########################
# create target for bulding the RF24Gateway lib
###########################
add_library(${LibTargetName} SHARED RF24Gateway.cpp)
target_include_directories(${LibTargetName} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${LibTargetName} INTERFACE
    project_options
    project_warnings
    SHARED ${RF24}
    SHARED ${RF24Network}
    SHARED ${RF24Mesh}
)

set_target_properties(
    ${LibTargetName}
    PROPERTIES
    SOVERSION ${${LibName}_VERSION_MAJOR}
    VERSION ${${LibName}_VERSION_STRING}
)

# conditionally disable interruot support (a pigpio specific feature)
if("${LibPIGPIO}" STREQUAL "LibPIGPIO-NOTFOUND" OR DEFINED RF24_NO_INTERRUPT)
    message(STATUS "Disabling IRQ pin support")
    target_compile_definitions(${LibTargetName} PUBLIC RF24_NO_INTERRUPT)
endif()

# assert the appropriate preprocessor macros for RF24Network_config.h
if(RF24GATEWAY_DEBUG_LEVEL GREATER 0)
    message(STATUS "RF24GATEWAY_DEBUG_LEVEL set to ${RF24GATEWAY_DEBUG_LEVEL}")
    target_compile_definitions(${LibTargetName} PUBLIC RF24GATEWAY_DEBUG_LEVEL=${RF24GATEWAY_DEBUG_LEVEL})
endif()

###########################
# target install rules for the RF24Gateway lib
###########################
install(TARGETS ${LibTargetName} DESTINATION lib)

install(FILES
        RF24Gateway.h
    DESTINATION include/RF24Gateway
)

# CMAKE_CROSSCOMPILING is only TRUE when CMAKE_TOOLCHAIN_FILE is specified via CLI
if(CMAKE_HOST_UNIX AND "${CMAKE_CROSSCOMPILING}" STREQUAL "FALSE")
    install(CODE "message(STATUS \"Updating ldconfig\")")
    install(CODE "execute_process(COMMAND ldconfig)")
endif()
