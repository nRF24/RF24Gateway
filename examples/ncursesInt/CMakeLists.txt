# this example needs the ncurses package installed
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(example RF24Gateway_ncursesInt)

# make a target
add_executable(${example} ${example}.cpp)

# link the RF24 lib to the target.
target_link_libraries(${example} PUBLIC
    ${linked_libs}
    ${CURSES_LIBRARIES}
)
