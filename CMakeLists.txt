cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SofaFrontEndViewer")
project(${PROJECT_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/functions.cmake)

set(LIBRARIES_BASE_DIR "" CACHE PATH "Directory where other libraries are installed")
set(LIBRARIES_INCLUDE_DIRS ${LIBRARIES_BASE_DIR}/include)
set(LIBRARIES_LIB_DIRS ${LIBRARIES_BASE_DIR}/lib)

include_directories(".")
include_directories(${LIBRARIES_INCLUDE_DIRS})

set(TARGET_DIR ${CMAKE_INSTALL_PREFIX}/\${BUILD_TYPE})

# Multiprocessor build on Visual C++
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Librairies
add_subdirectory("core")
add_subdirectory("render")

# Plugins
add_subdirectory("modules/SFELocal")
add_subdirectory("modules/ModelViewer")

# Application
add_subdirectory("ui")
