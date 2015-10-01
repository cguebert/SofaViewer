cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "ModelViewer")
project(${PROJECT_NAME})

set(HEADER_FILES
	Document.h
)

set(SOURCE_FILES
	Document.cpp
)

set(MODULE_DIR "${TARGET_DIR}/modules/${PROJECT_NAME}")

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Plugins")

install(TARGETS ${PROJECT_NAME} DESTINATION ${MODULE_DIR})

# Dependencies
target_link_libraries(${PROJECT_NAME} "Core")
target_link_libraries(${PROJECT_NAME} "SimpleRender")

add_library("Assimp" SHARED IMPORTED)
set_target_properties("Assimp" PROPERTIES IMPORTED_IMPLIB ${LIBRARIES_BASE_DIR}/lib/assimp.lib)
target_link_libraries(${PROJECT_NAME} "Assimp")

install(FILES ${LIBRARIES_BASE_DIR}/lib/assimp.dll DESTINATION ${MODULE_DIR})