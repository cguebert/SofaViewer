cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SimpleRender")
project(${PROJECT_NAME})

set(HEADER_FILES
	Model.h
	Scene.h
	Shader.h
)

set(SOURCE_FILES
	Model.cpp
	Scene.cpp
	Shader.cpp
)

include_directories(${LIBRARIES_BASE_DIR}/include)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)
set(TARGETDIR ${LIBRARY_OUTPUT_PATH}/\${BUILD_TYPE})

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})
