cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Serialization")
project(${PROJECT_NAME})

set(HEADER_FILES
	serialization.h
	DocXML.h
)

set(SOURCE_FILES
	ExportDocXML.cpp
	ImportDocXML.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} "Core")

add_library("TinyXML2" SHARED IMPORTED)
set_target_properties("TinyXML2" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/tinyxml2.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/tinyxml2d.lib)
target_link_libraries(${PROJECT_NAME} "TinyXML2")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SHARED_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraries")

add_definitions(-DBUILD_SERIALIZATION)
