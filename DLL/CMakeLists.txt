cmake_minimum_required(VERSION 3.24)
project(GardenGate VERSION 0.1.2 LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  minhook
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
  GIT_TAG 1e9ad1eb42db11bfcb65461f687c656612d1b555
)

FetchContent_MakeAvailable(minhook)

add_library(${PROJECT_NAME} SHARED)

# Language / standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

# Sources (project only)
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

target_sources(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCES}
)

# Includes
target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Warnings (Add back -Werror when those get fixed)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
)

# Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ws2_32
    minhook
  )

# Output
set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "dinput8"
)
if (MINGW)
    set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    IMPORT_PREFIX ""
  )
endif()
