cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Core")
project(${PROJECT_NAME})

set(HEADER_FILES
	BaseDocument.h
	DocumentFactory.h
	Graph.h
	MouseEvent.h
	MouseManipulator.h
	ObjectProperties.h
	Property.h
	SimpleGUI.h
	VectorWrapper.h
)

set(SOURCE_FILES
	DocumentFactory.cpp
	Graph.cpp
	MouseManipulator.cpp
	ObjectProperties.cpp
	Property.cpp
)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})
