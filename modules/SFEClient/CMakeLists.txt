cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SFEClient")
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
target_link_libraries(${PROJECT_NAME} "SFELib")

# List of all sofa libs.
set(SOFA_LIBS )

if(WIN32)
	add_library("SofaFrontEnd" SHARED IMPORTED)
	set_target_properties("SofaFrontEnd" PROPERTIES 
		IMPORTED_IMPLIB ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0.lib
		IMPORTED_IMPLIB_RELEASE ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0.lib
		IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0rd.lib
		IMPORTED_IMPLIB_DEBUG ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0d.lib)
	target_link_libraries(${PROJECT_NAME} "SofaFrontEnd")
	
	add_library("SofaFrontEndClient" SHARED IMPORTED)
	set_target_properties("SofaFrontEndClient" PROPERTIES 
		IMPORTED_IMPLIB ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndClient_1_0.lib
		IMPORTED_IMPLIB_RELEASE ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndClient_1_0.lib
		IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndClient_1_0rd.lib
		IMPORTED_IMPLIB_DEBUG ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndClient_1_0d.lib)
	target_link_libraries(${PROJECT_NAME} "SofaFrontEndClient")
	
	list(APPEND SOFA_LIBS SofaFrontEnd SofaFrontEndClient)
elseif(UNIX)	
	if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif()

	set_target_properties(${target} PROPERTIES
		BUILD_WITH_INSTALL_RPATH 1
		INSTALL_RPATH ".")
	set(dependent_libs)
	foreach(lib ${SOFA_LIBS})
		add_library(${lib} SHARED IMPORTED)
		set_target_properties(${lib} PROPERTIES
			IMPORTED_LOCATION "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}"
			IMPORTED_LOCATION_RELEASE "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}"
			IMPORTED_LOCATION_RELWITHDEBINFO "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}rd${CMAKE_SHARED_LIBRARY_SUFFIX}"
			IMPORTED_LOCATION_DEBUG "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}d${CMAKE_SHARED_LIBRARY_SUFFIX}"
		)
		target_link_libraries(${target} ${lib})
	endforeach(lib)
endif()

##
# Installation of libraries.
##

# Under windows, we set a version on Sofa libraries.
if(WIN32)
	set(version "_1_0")
endif()

set(libNames)
foreach(lib ${SOFA_LIBS})
	list(APPEND libNames ${lib}${version})      
endforeach(lib)

set(libsRelease)
set(libsRelWithDebInfo)
set(libsDebug)
foreach(libName ${libNames})
	list(APPEND libsRelease         "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${libName}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	list(APPEND libsRelWithDebInfo  "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${libName}rd${CMAKE_SHARED_LIBRARY_SUFFIX}")
	list(APPEND libsDebug           "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${libName}d${CMAKE_SHARED_LIBRARY_SUFFIX}")
endforeach(libName)
install(FILES ${libsRelease} CONFIGURATIONS Release DESTINATION ${MODULE_DIR})
install(FILES ${libsRelWithDebInfo} CONFIGURATIONS RelWithDebInfo DESTINATION ${MODULE_DIR})
install(FILES ${libsDebug} CONFIGURATIONS Debug DESTINATION ${MODULE_DIR})

