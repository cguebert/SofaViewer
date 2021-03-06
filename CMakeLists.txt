cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SofaFrontEndViewer")
project(${PROJECT_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/functions.cmake)

set(LIBRARIES_BASE_DIR "" CACHE PATH "Directory where other libraries are installed")
set(LIBRARIES_INCLUDE_DIR ${LIBRARIES_BASE_DIR}/include)
set(LIBRARIES_LIB_DIR ${LIBRARIES_BASE_DIR}/lib)

include_directories(".")
include_directories(${LIBRARIES_INCLUDE_DIR})

set(TARGET_DIR "${CMAKE_INSTALL_PREFIX}/\${BUILD_TYPE}")
set(SHARED_DIR "${TARGET_DIR}/shared")

# Multiprocessor build on Visual C++
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Librairies
include_directories("libs")

add_subdirectory("libs/core")
add_subdirectory("libs/render")
add_subdirectory("libs/serialization")
add_subdirectory("libs/SFELib")

include_directories("modules")

# Plugins
add_subdirectory("modules/SFELocal")
add_subdirectory("modules/SFEClient")
add_subdirectory("modules/SGA")

# Application
add_subdirectory("ui")
