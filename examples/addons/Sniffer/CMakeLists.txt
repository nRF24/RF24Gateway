set(example Sniffer)

# make a target
add_executable(${example} ${example}.cpp)

# link the RF24 lib to the target.
target_link_libraries(${example} PUBLIC
    ${linked_libs}
)
if("${LibPIGPIO}" STREQUAL "LibPIGPIO-NOTFOUND" OR DEFINED RF24_NO_INTERRUPT)
    target_compile_definitions(${example} PUBLIC RF24_NO_INTERRUPT)
endif()
