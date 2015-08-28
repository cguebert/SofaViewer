cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SimpleRender")
project(${PROJECT_NAME})

set(HEADER_FILES
	Model.h
	Scene.h
	Shader.h
	shaders.h
)

set(SOURCE_FILES
	Model.cpp
	Scene.cpp
	Shader.cpp
)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} "${LIBRARIES_LIB_DIRS}/glew32s.lib")

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
 
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraries")
