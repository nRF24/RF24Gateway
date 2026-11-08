cmake_minimum_required(VERSION 3.15)

project(rf24gatewayd C CXX)
include(GNUInstallDirs)
add_compile_options(-Ofast -Wall)

set(RF24GATEWAY_USER "rf24gateway" CACHE STRING "Unprivileged user the rf24gateway service runs as")
set(RF24GATEWAY_SERVICE_MODE "ncurses" CACHE STRING
    "headless: no interface, logs to journald. ncurses: GUI in a screen session. ncursesint: GUI using the radio's IRQ pin")
set_property(CACHE RF24GATEWAY_SERVICE_MODE PROPERTY STRINGS headless ncurses ncursesint)
if(NOT RF24GATEWAY_SERVICE_MODE MATCHES "^(headless|ncurses|ncursesint)$")
    message(FATAL_ERROR "RF24GATEWAY_SERVICE_MODE must be headless, ncurses or ncursesint")
endif()
option(RF24NETWORK_CCM "build rf24gatewayd with AES-CCM encryption support (RF24Network, RF24Mesh and RF24Gateway must be built with it too)" OFF)
message(STATUS "rf24gatewayd mode: ${RF24GATEWAY_SERVICE_MODE}, user: ${RF24GATEWAY_USER}, CCM: ${RF24NETWORK_CCM}")

include(../cmake/detectCPU.cmake)
include(../cmake/AutoConfig_RF24_DRIVER.cmake)

find_library(RF24 rf24 REQUIRED)
find_library(RF24Network rf24network REQUIRED)
find_library(RF24Mesh rf24mesh REQUIRED)
find_library(RF24Gateway rf24gateway REQUIRED)

set(linked_libs ${RF24} pthread ${RF24Network} ${RF24Mesh} ${RF24Gateway})
if("${RF24_DRIVER}" STREQUAL "pigpio" AND NOT "${LibPIGPIO}" STREQUAL "LibPIGPIO-NOTFOUND")
    message(WARNING "pigpio needs root, so the service can't run as ${RF24GATEWAY_USER}. Use the SPIDEV driver")
    list(APPEND linked_libs ${LibPIGPIO})
endif()

add_executable(rf24gatewayd rf24gatewayd_${RF24GATEWAY_SERVICE_MODE}.cpp)
target_compile_features(rf24gatewayd PRIVATE cxx_std_17)
target_link_libraries(rf24gatewayd PRIVATE ${linked_libs})
if(RF24NETWORK_CCM)
    find_file(RF24CCM_HEADER RF24Network/RF24CCM.h)
    if(NOT RF24CCM_HEADER)
        message(FATAL_ERROR "RF24NETWORK_CCM is ON, but RF24Network/RF24CCM.h is not installed. "
            "Install RF24Network, RF24Mesh and RF24Gateway built with -DRF24NETWORK_CCM=ON first")
    endif()
    target_compile_definitions(rf24gatewayd PRIVATE RF24NETWORK_CCM)
endif()

set(daemon "${CMAKE_INSTALL_FULL_BINDIR}/rf24gatewayd /etc/rf24gateway/rf24gateway.conf")
if(RF24GATEWAY_SERVICE_MODE STREQUAL "headless")
    set(SERVICE_EXEC_LINES "# Output goes to journald: journalctl -u rf24gateway -f\nExecStart=${daemon}")
else()
    find_package(Curses REQUIRED) # apt install libncurses-dev
    target_include_directories(rf24gatewayd PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(rf24gatewayd PRIVATE ${CURSES_LIBRARIES})
    set(SERVICE_EXEC_LINES "# -D -m: start screen detached without forking, so systemd tracks it.
# View the UI with: rf24gateway-ui   (detach: Ctrl-A then D)
ExecStart=/usr/bin/screen -D -m -S rf24gateway ${daemon}
Environment=TERM=xterm-256color")
    configure_file(rf24gateway-ui.in ${CMAKE_CURRENT_BINARY_DIR}/rf24gateway-ui @ONLY)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rf24gateway-ui DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

configure_file(rf24gateway.service.in ${CMAKE_CURRENT_BINARY_DIR}/rf24gateway.service @ONLY)
configure_file(rf24gateway.sysusers.in ${CMAKE_CURRENT_BINARY_DIR}/sysusers/rf24gateway.conf @ONLY)

install(TARGETS rf24gatewayd DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rf24gateway.service DESTINATION /etc/systemd/system)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysusers/rf24gateway.conf DESTINATION /etc/sysusers.d)
# Install the sample config only if there isn't one already, so upgrades keep the user's settings
install(CODE "
    if(NOT EXISTS \$ENV{DESTDIR}/etc/rf24gateway/rf24gateway.conf)
        file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/rf24gateway.conf DESTINATION /etc/rf24gateway)
    endif()
")
if(RF24GATEWAY_SERVICE_MODE STREQUAL "headless")
    # A previous GUI install's attach helper would point at a screen session that no longer exists
    install(CODE "file(REMOVE \$ENV{DESTDIR}${CMAKE_INSTALL_FULL_BINDIR}/rf24gateway-ui)")
endif()
# Create the service user now, on a real install (not when staging a package with DESTDIR)
install(CODE "
    if(\"\$ENV{DESTDIR}\" STREQUAL \"\")
        execute_process(COMMAND systemd-sysusers /etc/sysusers.d/rf24gateway.conf)
    endif()
")
