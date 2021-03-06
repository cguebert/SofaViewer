cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SFELocal")
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
set(SOFA_LIBS
	Qwt SofaBaseAnimationLoop SofaBaseCollision SofaBaseLinearSolver SofaBaseMechanics SofaBaseTopology SofaBaseVisual SofaBoundaryCondition SofaComponentAdvanced SofaComponentBase SofaComponentCommon SofaComponentGeneral SofaComponentMain SofaComponentMisc SofaConstraint SofaCore SofaDefaultType SofaDeformable SofaDenseSolver SofaEigen2Solver SofaEngine SofaEulerianFluid SofaExplicitOdeSolver SofaExporter SofaGraphComponent SofaGuiCommon SofaGuiGlut SofaGuiMain SofaGuiQt SofaHaptics SofaHelper SofaImplicitOdeSolver SofaLoader SofaMeshCollision SofaMiscCollision SofaMiscEngine SofaMiscFem SofaMiscForceField SofaMiscMapping SofaMiscSolver SofaMiscTopology SofaMisc SofaNonUniformFem SofaObjectInteraction SofaOpenglVisual SofaPreconditioner SofaRigid SofaSimpleFem SofaSimulationCommon SofaSimulationGraph SofaSimulationTree SofaSphFluid SofaTopologyMapping SofaUserInteraction SofaValidation SofaVolumetricData tinyxml
)

if(WIN32)
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
	
	add_library("SofaFrontEndServer" SHARED IMPORTED)
	set_target_properties("SofaFrontEndServer" PROPERTIES 
		IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/SofaFrontEndServer_1_0.lib
		IMPORTED_IMPLIB_RELEASE ${LIBRARIES_LIB_DIR}/SofaFrontEndServer_1_0.lib
		IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_LIB_DIR}/SofaFrontEndServer_1_0rd.lib
		IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/SofaFrontEndServer_1_0d.lib)
	target_link_libraries(${PROJECT_NAME} "SofaFrontEndServer")
	
	add_library("SofaFrontEndCommunication" STATIC IMPORTED)
	set_target_properties("SofaFrontEndCommunication" PROPERTIES 
		IMPORTED_LOCATION ${LIBRARIES_LIB_DIR}/SofaFrontEndCommunication_1_0.lib
		IMPORTED_LOCATION_RELEASE ${LIBRARIES_LIB_DIR}/SofaFrontEndCommunication_1_0.lib
		IMPORTED_LOCATION_RELWITHDEBINFO ${LIBRARIES_LIB_DIR}/SofaFrontEndCommunication_1_0rd.lib
		IMPORTED_LOCATION_DEBUG ${LIBRARIES_LIB_DIR}/SofaFrontEndCommunication_1_0d.lib)
	target_link_libraries(${PROJECT_NAME} "SofaFrontEndCommunication")
	
	list(APPEND SOFA_LIBS SofaFrontEnd SofaFrontEndLocal SofaFrontEndServer)
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
			IMPORTED_LOCATION "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}"
			IMPORTED_LOCATION_RELEASE "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}"
			IMPORTED_LOCATION_RELWITHDEBINFO "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}rd${CMAKE_SHARED_LIBRARY_SUFFIX}"
			IMPORTED_LOCATION_DEBUG "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}d${CMAKE_SHARED_LIBRARY_SUFFIX}"
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
	list(APPEND libsRelease         "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${libName}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	list(APPEND libsRelWithDebInfo  "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${libName}rd${CMAKE_SHARED_LIBRARY_SUFFIX}")
	list(APPEND libsDebug           "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${libName}d${CMAKE_SHARED_LIBRARY_SUFFIX}")
endforeach(libName)
install(FILES ${libsRelease} CONFIGURATIONS Release DESTINATION ${SHARED_DIR})
install(FILES ${libsRelWithDebInfo} CONFIGURATIONS RelWithDebInfo DESTINATION ${SHARED_DIR})
install(FILES ${libsDebug} CONFIGURATIONS Debug DESTINATION ${SHARED_DIR})

# Install directive for additional sofa dependencies that do not depend on the configuration.
if(WIN32)
	set(SOFA_ADDITIONAL_LIBS glew32 glut32 iconv libpng15 libxml2 zlib1)
	set(libs)
	foreach(lib ${SOFA_ADDITIONAL_LIBS})
		set(libfullpath "${LIBRARIES_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		list(APPEND libs "${libfullpath} ")
	endforeach(lib)	
	install(FILES ${libs} DESTINATION ${SHARED_DIR})
endif()

