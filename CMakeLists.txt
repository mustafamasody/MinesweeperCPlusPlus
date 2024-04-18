#cmake_minimum_required(VERSION 3.27)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
#project(Project3Minesweeper)
#
### If you want to link SFML statically
## set(SFML_STATIC_LIBRARIES TRUE)
#
### In most cases better set in the CMake cache
#set(SFML_DIR "/Users/mustafa/Downloads/SFML-2.6.1-macOS-clang-arm64")
#
#find_package(SFML 2.5.1 COMPONENTS system window graphics audio REQUIRED)
#add_executable(Project3Minesweeper main.cpp)
#target_link_libraries(Project3Minesweeper sfml-system sfml-window sfml-graphics sfml-audio)

cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Project3Minesweeper)

## If you want to link SFML statically
# set(SFML_STATIC_LIBRARIES TRUE)

## In most cases better set in the CMake cache
set(SFML_DIR "/Users/mustafa/Downloads/SFML-2.6.1-macOS-clang-arm64")

find_package(SFML 2.5.1 COMPONENTS system window graphics audio REQUIRED)
add_executable(Project3Minesweeper main.cpp
        Tile.h
        WelcomeWindow.h
        Game.h
        SpriteManager.h
)
target_link_libraries(Project3Minesweeper sfml-system sfml-window sfml-graphics sfml-audio)

# Define the source directory and the build directory
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_BINARY_DIR})

# Copy the files directory from the source directory to the build directory
file(COPY ${SOURCE_DIR}/files DESTINATION ${BUILD_DIR})