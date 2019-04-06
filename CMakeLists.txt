list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 2.6)
project(feathers CXX)

# main sources and headers
set(HEADER_DIRECTORY "include")
set(SOURCE_DIRECTORY "source")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -W -fdiagnostics-color=always -Wall -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wdisabled-optimization -Wduplicated-cond -Wextra -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wno-sign-conversion -Wnoexcept -Wnull-dereference -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wstrict-aliasing=1 -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wno-unknown-pragmas -Wuseless-cast -Wno-parentheses -Wno-unused-local-typedefs")

file(GLOB_RECURSE SOURCES_FILE "${SOURCE_DIRECTORY}/*.cpp")
file(GLOB_RECURSE HEADERS_FILE "${HEADER_DIRECTORY}/*.hpp")

find_package(Wayland REQUIRED)
find_package(Wlroots REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Pixman REQUIRED)

include_directories(
  ${HEADER_DIRECTORY}
  ${WAYLAND_CLIENT_INCLUDE_DIR}
  ${WAYLAND_SERVER_INCLUDE_DIR}
  ${WLR_INCLUDE_DIRS}
  ${PIXMAN_INCLUDE_DIRS}
  )

add_executable(
  ${PROJECT_NAME}
  ${SOURCES_FILE}
  ${HEADERS_FILE}
)

target_link_libraries(${PROJECT_NAME}
  ${WAYLAND_CLIENT_LIBRARIES}
  ${WAYLAND_SERVER_LIBRARIES}
  ${WLR_LIBRARIES}
  ${PIXMAN_LIBRARIES}
  xkbcommon
  )
