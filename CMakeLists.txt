cmake_minimum_required(VERSION 3.18)

project(minesweeper.net VERSION 0.1)

# -std=c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# -D flag:
# add_definitions(-DSOME_IMPORTANT_DEFINITION)

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic-errors")

find_package(SFML 2.5 COMPONENTS system graphics window network REQUIRED)

add_library(field
    src/field.cpp
    src/include/field_cell.hpp
    src/include/field_base.hpp)

add_library(graph
    src/graphic.cpp
    src/include/graphic.hpp)

add_executable(mines_sp
    src/mine_sweeper_standalone)

add_executable(mines_server
    src/mine_server)

add_executable(mines_client
    src/field_client
    src/mine_client)

target_link_libraries(mines_sp
    graph
    field
    sfml-system
    sfml-graphics
    sfml-window)

target_link_libraries(mines_server
    field
    sfml-system
    sfml-network)

target_link_libraries(mines_client
    graph
    sfml-system
    sfml-graphics
    sfml-window
    sfml-network)


install(TARGETS mines_sp EXPORT mines_spConfig)
install(DIRECTORY resources DESTINATION bin)

# install(TARGETS mines_sp mines_server mines_client
#     # DESTINATION "resources"
#     CONFIGURATIONS Release
#     # RUNTIME DESTINATION bin
# )
