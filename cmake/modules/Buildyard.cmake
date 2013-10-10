
# Bootstrap Buildyard from a project source directory.
#
# Needs CMake/<project>.cmake and CMake/depends.txt, which are generated by
# Buildyard. git add them to your project and then add to the top-level
# CMakeLists.txt:
#   project(Foo)
#   list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
#   if(NOT BUILDYARD)
#     include(Buildyard)
#     return()
#   endif()
#   ...

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
if(BUILDYARD)
  message(FATAL_ERROR "Running from within Buildyard, don't use me here")
endif()

include(GitExternal)
include(UpdateFile)

git_external("${CMAKE_BINARY_DIR}/Buildyard"
  https://github.com/Eyescale/Buildyard.git master)

update_file("${CMAKE_SOURCE_DIR}/CMake/${CMAKE_PROJECT_NAME}.cmake"
  "${CMAKE_BINARY_DIR}/Buildyard/config.Buildyard/${CMAKE_PROJECT_NAME}.cmake")
update_file("${CMAKE_SOURCE_DIR}/CMake/depends.txt"
  "${CMAKE_BINARY_DIR}/Buildyard/config.Buildyard/depends.txt")

set(BUILDYARD_TARGETS ${CMAKE_PROJECT_NAME})
add_subdirectory("${CMAKE_BINARY_DIR}/Buildyard" # source
  "${CMAKE_BINARY_DIR}/Buildyard.bin" )          # binary
if(MSVC)
  message(STATUS "Build 00_Main->AllProjects to bootstrap and 00_Main->AllBuild after bootstrap\n")
else()
  message(STATUS "Use 'make -jN' to bootstrap and 'make -jN AllBuild' after bootstrap\n")
endif()