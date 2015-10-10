cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SFELib")
project(${PROJECT_NAME})

set(HEADER_FILES
	GraphImages.h
	SofaDocument.h
	SofaProperties.h
)

set(SOURCE_FILES
	GraphImages.cpp
	SofaDocument.cpp
	SofaProperties.cpp
)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraries")

# Dependencies
target_link_libraries(${PROJECT_NAME} "Core")
target_link_libraries(${PROJECT_NAME} "SimpleRender")

if(WIN32)
	add_library("SofaFrontEnd" SHARED IMPORTED)
	set_target_properties("SofaFrontEnd" PROPERTIES 
		IMPORTED_IMPLIB ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0.lib
		IMPORTED_IMPLIB_RELEASE ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0.lib
		IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0rd.lib
		IMPORTED_IMPLIB_DEBUG ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0d.lib)
	target_link_libraries(${PROJECT_NAME} "SofaFrontEnd")
elseif(UNIX)	
	if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif()
endif()
