cmake_minimum_required(VERSION 3.20)

project(MOI3_5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EMBREE_ROOT "${CMAKE_SOURCE_DIR}/embree-3.13.5.x64.vc14.windows")
set(EMBREE_INCLUDE_DIR "${EMBREE_ROOT}/include")
set(EMBREE_LIB_DIR "${EMBREE_ROOT}/lib")
set(EMBREE_BIN_DIR "${EMBREE_ROOT}/bin")

if(NOT EXISTS "${EMBREE_INCLUDE_DIR}/embree3/rtcore.h")
  message(FATAL_ERROR "Embree headers were not found in ${EMBREE_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${EMBREE_LIB_DIR}/embree3.lib")
  message(FATAL_ERROR "Embree library was not found in ${EMBREE_LIB_DIR}")
endif()

add_executable(renderlab
  main.cpp
  renderer.cpp
  scene_config.cpp
  ppm.cpp
)

target_include_directories(renderlab PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${EMBREE_INCLUDE_DIR}"
)

target_link_directories(renderlab PRIVATE "${EMBREE_LIB_DIR}")
target_link_libraries(renderlab PRIVATE embree3 tbb)

set_target_properties(renderlab PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_custom_command(TARGET renderlab POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${EMBREE_BIN_DIR}/embree3.dll"
          "$<TARGET_FILE_DIR:renderlab>/embree3.dll"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${EMBREE_BIN_DIR}/tbb12.dll"
          "$<TARGET_FILE_DIR:renderlab>/tbb12.dll"
)
