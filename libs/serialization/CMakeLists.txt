cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Serialization")
project(${PROJECT_NAME})

set(HEADER_FILES
	serialization.h
	DocXML.h
)

set(SOURCE_FILES
	DocXML.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} "Core")
target_link_libraries(${PROJECT_NAME} "${LIBRARIES_LIB_DIRS}/tinyxml2.lib")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SHARED_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraries")

add_definitions(-DBUILD_SERIALIZATION)
