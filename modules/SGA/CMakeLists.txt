cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SGA")
project(${PROJECT_NAME})

set(HEADER_FILES
	Document.h
	MeshImport.h
	SGAExecution.h
	SGAProperties.h
)

set(SOURCE_FILES
	Document.cpp
	MeshImport.cpp
	SGAExecution.cpp
	SGAProperties.cpp
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

add_library("SofaGraphAbstraction" SHARED IMPORTED)
set_target_properties("SofaGraphAbstraction" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_BASE_DIR}/lib/SofaGraphAbstraction.lib
	IMPORTED_IMPLIB_RELEASE ${LIBRARIES_BASE_DIR}/lib/SofaGraphAbstraction.lib
	IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_BASE_DIR}/lib/SofaGraphAbstractionrd.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_BASE_DIR}/lib/SofaGraphAbstractiond.lib)
target_link_libraries(${PROJECT_NAME} "SofaGraphAbstraction")
install(FILES ${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SofaGraphAbstraction${CMAKE_SHARED_LIBRARY_SUFFIX} CONFIGURATIONS Release DESTINATION ${MODULE_DIR})
install(FILES ${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SofaGraphAbstractionrd${CMAKE_SHARED_LIBRARY_SUFFIX} CONFIGURATIONS RelWithDebInfo DESTINATION ${MODULE_DIR})
install(FILES ${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SofaGraphAbstractiond${CMAKE_SHARED_LIBRARY_SUFFIX} CONFIGURATIONS Debug DESTINATION ${MODULE_DIR})

install(DIRECTORY ${LIBRARIES_BASE_DIR}/sga/definitions DESTINATION ${MODULE_DIR})

install(FILES ${LIBRARIES_BASE_DIR}/lib/assimp.dll DESTINATION ${SHARED_DIR})