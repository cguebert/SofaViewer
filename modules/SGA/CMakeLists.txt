cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SGA")
project(${PROJECT_NAME})

set(HEADER_FILES
	MeshDocument.h
	MeshImport.h
	SGADocument.h
	SGAExecution.h
	SGAProperties.h
)

set(SOURCE_FILES
	MeshDocument.cpp
	MeshImport.cpp
	SGADocument.cpp
	SGAExecution.cpp
	SGAProperties.cpp
)

set(MODULE_DIR "${TARGET_DIR}/modules/${PROJECT_NAME}")

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Plugins")

install(TARGETS ${PROJECT_NAME} DESTINATION ${MODULE_DIR})

# Dependencies
target_link_libraries(${PROJECT_NAME} "Core" "Serialization" )
target_link_libraries(${PROJECT_NAME} "SimpleRender")

add_library("Assimp" SHARED IMPORTED)
set_target_properties("Assimp" PROPERTIES IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/assimp.lib)
target_link_libraries(${PROJECT_NAME} "Assimp")

add_library("SofaGraphAbstraction" SHARED IMPORTED)
set_target_properties("SofaGraphAbstraction" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/SofaGraphAbstraction.lib
	IMPORTED_IMPLIB_RELEASE ${LIBRARIES_LIB_DIR}/SofaGraphAbstraction.lib
	IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_LIB_DIR}/SofaGraphAbstractionrd.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/SofaGraphAbstractiond.lib)
target_link_libraries(${PROJECT_NAME} "SofaGraphAbstraction")
install(FILES ${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}SofaGraphAbstraction${CMAKE_SHARED_LIBRARY_SUFFIX} CONFIGURATIONS Release DESTINATION ${MODULE_DIR})
install(FILES ${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}SofaGraphAbstractionrd${CMAKE_SHARED_LIBRARY_SUFFIX} CONFIGURATIONS RelWithDebInfo DESTINATION ${MODULE_DIR})
install(FILES ${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}SofaGraphAbstractiond${CMAKE_SHARED_LIBRARY_SUFFIX} CONFIGURATIONS Debug DESTINATION ${MODULE_DIR})

add_library("SofaFrontEnd" SHARED IMPORTED)
set_target_properties("SofaFrontEnd" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/SofaFrontEnd_1_0.lib
	IMPORTED_IMPLIB_RELEASE ${LIBRARIES_LIB_DIR}/SofaFrontEnd_1_0.lib
	IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_LIB_DIR}/SofaFrontEnd_1_0rd.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/SofaFrontEnd_1_0d.lib)
target_link_libraries(${PROJECT_NAME} "SofaFrontEnd")
	
add_library("SofaFrontEndLocal" SHARED IMPORTED)
set_target_properties("SofaFrontEndLocal" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/SofaFrontEndLocal_1_0.lib
	IMPORTED_IMPLIB_RELEASE ${LIBRARIES_LIB_DIR}/SofaFrontEndLocal_1_0.lib
	IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_LIB_DIR}/SofaFrontEndLocal_1_0rd.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/SofaFrontEndLocal_1_0d.lib)
target_link_libraries(${PROJECT_NAME} "SofaFrontEndLocal")

install(DIRECTORY ${LIBRARIES_BASE_DIR}/sga/definitions DESTINATION ${MODULE_DIR})

install(FILES ${LIBRARIES_LIB_DIR}/assimp.dll DESTINATION ${SHARED_DIR})