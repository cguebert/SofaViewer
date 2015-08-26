cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SFELocal")
project(${PROJECT_NAME})

set(HEADER_FILES
	Document.h
	GraphImages.h
	SofaProperties.h
)

set(SOURCE_FILES
	Document.cpp
	GraphImages.cpp
	SofaProperties.cpp
)

include_directories(${LIBRARIES_BASE_DIR}/include)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)
set(TARGETDIR ${EXECUTABLE_OUTPUT_PATH}/\${BUILD_TYPE})

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

# List of all sofa libs.
set(SOFA_LIBS
	Qwt SofaBaseAnimationLoop SofaBaseCollision SofaBaseLinearSolver SofaBaseMechanics SofaBaseTopology SofaBaseVisual SofaBoundaryCondition SofaComponentAdvanced SofaComponentBase SofaComponentCommon SofaComponentGeneral SofaComponentMain SofaComponentMisc SofaConstraint SofaCore SofaDefaultType SofaDeformable SofaDenseSolver SofaEigen2Solver SofaEngine SofaEulerianFluid SofaExplicitOdeSolver SofaExporter SofaGraphComponent SofaGuiCommon SofaGuiGlut SofaGuiMain SofaGuiQt SofaHaptics SofaHelper SofaImplicitOdeSolver SofaLoader SofaMeshCollision SofaMiscCollision SofaMiscEngine SofaMiscFem SofaMiscForceField SofaMiscMapping SofaMiscSolver SofaMiscTopology SofaMisc SofaNonUniformFem SofaObjectInteraction SofaOpenglVisual SofaPreconditioner SofaRigid SofaSimpleFem SofaSimulationCommon SofaSimulationGraph SofaSimulationTree SofaSphFluid SofaTopologyMapping SofaUserInteraction SofaValidation SofaVolumetricData tinyxml
)

if(WIN32)
	add_library("SofaFrontEnd" SHARED IMPORTED)
	set_target_properties("SofaFrontEnd" PROPERTIES 
		IMPORTED_IMPLIB ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0.lib
		IMPORTED_IMPLIB_RELEASE ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0.lib
		IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0rd.lib
		IMPORTED_IMPLIB_DEBUG ${LIBRARIES_BASE_DIR}/lib/SofaFrontEnd_1_0d.lib)
	target_link_libraries(${PROJECT_NAME} "SofaFrontEnd")
	
	add_definitions(-DSFE_USE_LOCAL)
	list(APPEND SOFA_LIBS SofaFrontEnd SofaFrontEndLocal)
	
	add_library("SofaFrontEndLocal" SHARED IMPORTED)
	set_target_properties("SofaFrontEndLocal" PROPERTIES 
		IMPORTED_IMPLIB ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndLocal_1_0.lib
		IMPORTED_IMPLIB_RELEASE ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndLocal_1_0.lib
		IMPORTED_IMPLIB_RELWITHDEBINFO ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndLocal_1_0rd.lib
		IMPORTED_IMPLIB_DEBUG ${LIBRARIES_BASE_DIR}/lib/SofaFrontEndLocal_1_0d.lib)
	target_link_libraries(${PROJECT_NAME} "SofaFrontEndLocal")
elseif(UNIX)	
	if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif()

	list(APPEND INC_SYS
		${SOFA_INCLUDE_DIRS}
	)
	
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
install(FILES ${libsRelease} CONFIGURATIONS Release DESTINATION ${TARGETDIR})
install(FILES ${libsRelWithDebInfo} CONFIGURATIONS RelWithDebInfo DESTINATION ${TARGETDIR})
install(FILES ${libsDebug} CONFIGURATIONS Debug DESTINATION ${TARGETDIR})

# Install directive for additional sofa dependencies that do not depend on the configuration.
if(WIN32)
	set(SOFA_ADDITIONAL_LIBS glew32 glut32 iconv libpng15 libxml2 zlib1)
	set(libs)
	foreach(lib ${SOFA_ADDITIONAL_LIBS})
		set(libfullpath "${LIBRARIES_BASE_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		list(APPEND libs "${libfullpath} ")
	endforeach(lib)	
	install(FILES ${libs} CONFIGURATIONS Release RelWithDebInfo Debug DESTINATION ${TARGETDIR})
endif()

