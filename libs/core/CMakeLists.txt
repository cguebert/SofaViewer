cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Core")
project(${PROJECT_NAME})

set(HEADER_FILES
	core.h
	BaseDocument.h
	DocumentFactory.h
	Graph.h
	GraphImage.h
	MetaProperties.h
	MouseEvent.h
	MouseManipulator.h
	ObjectProperties.h
	Property.h
	PropertiesUtils.h
	SimpleGUI.h
	StringConversion.h
	StructMeta.h
	VectorWrapper.h
)

set(SOURCE_FILES
	BaseDocument.cpp
	DocumentFactory.cpp
	Graph.cpp
	GraphImage.cpp
	MouseManipulator.cpp
	ObjectProperties.cpp
	Property.cpp
	SimpleGUI.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${TARGET_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraries")

add_definitions(-DBUILD_CORE)
