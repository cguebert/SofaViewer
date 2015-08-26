cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SofaFrontEndViewer")
project(${PROJECT_NAME})

set(QT_BASE_DIR "" CACHE PATH "Qt5 install prefix") # C:/Qt/5/msvc2015/
list(APPEND CMAKE_PREFIX_PATH ${QT_BASE_DIR})

set(LIBRARIES_BASE_DIR "" CACHE PATH "Directory where other libraries are installed")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(HEADER_FILES
	core/BaseDocument.h
	core/DocumentFactory.h
	core/Graph.h
	core/Model.h
	core/MouseEvent.h
	core/MouseManipulator.h
	core/ObjectProperties.h
	core/Property.h
	core/Scene.h
	core/SimpleGUI.h
	core/VectorWrapper.h
	modules/SFELocal/Document.h
	modules/SFELocal/GraphImages.h
	modules/SFELocal/SofaProperties.h
	ui/GraphModel.h
	ui/MainWindow.h
	ui/OpenGLView.h
	ui/PropertiesDialog.h
	ui/SimpleGUIImpl.h
	ui/widget/PropertyWidget.h
	ui/widget/PropertyWidgetFactory.h
	ui/widget/SimplePropertyWidget.h
	ui/widget/TablePropertyWidget.h
)

set(SOURCE_FILES
	main.cpp
	core/DocumentFactory.cpp
	core/Graph.cpp
	core/Model.cpp
	core/MouseManipulator.cpp
	core/ObjectProperties.cpp
	core/Property.cpp
	core/Scene.cpp
	modules/SFELocal/Document.cpp
	modules/SFELocal/GraphImages.cpp
	modules/SFELocal/SofaProperties.cpp
	ui/GraphModel.cpp
	ui/MainWindow.cpp
	ui/OpenGLView.cpp
	ui/PropertiesDialog.cpp
	ui/SimpleGUIImpl.cpp
	ui/widget/NumericalPropertyWidget.cpp
	ui/widget/PropertyWidget.cpp
	ui/widget/PropertyWidgetFactory.cpp
	ui/widget/StringPropertyWidget.cpp
	ui/widget/TablePropertyWidget.cpp
)

set(RESSOURCE_FILES ${PROJECT_NAME}.qrc)
if(WIN32)
	set(RESSOURCE_FILES ${RESSOURCE_FILES} ${PROJECT_NAME}.rc)
endif()

find_package(Qt5Widgets)
find_package(Qt5OpenGL)

include_directories(${LIBRARIES_BASE_DIR}/include)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)
set(TARGETDIR ${EXECUTABLE_OUTPUT_PATH}/\${BUILD_TYPE})

if(WIN32)
	set(GUI_TYPE WIN32)
endif()

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${RESSOURCE_FILES})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

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

# Install Qt files
if(WIN32)
	set(QT_LIBS Qt5Core Qt5Gui Qt5OpenGL Qt5Widgets)
	set(QT_ADDITIONAL_LIBS icudt54 icuin54 icuuc54)
	
	set(qtLibsRelease)
	set(qtLibsDebug)
	
	foreach(libName ${QT_LIBS})
		list(APPEND qtLibsRelease	"${QT_BASE_DIR}/bin/${libName}.dll")
		list(APPEND qtLibsDebug		"${QT_BASE_DIR}/bin/${libName}d.dll")
	endforeach(libName)
	
	foreach(libName ${QT_ADDITIONAL_LIBS})
		list(APPEND qtLibsRelease	"${QT_BASE_DIR}/bin/${libName}.dll")
		list(APPEND qtLibsDebug		"${QT_BASE_DIR}/bin/${libName}.dll")
	endforeach(libName)
	
	install(FILES ${qtLibsRelease} CONFIGURATIONS Release RelWithDebInfo DESTINATION ${TARGETDIR})
	install(FILES ${qtLibsDebug} CONFIGURATIONS Debug DESTINATION ${TARGETDIR})
	
	install(FILES "${QT_BASE_DIR}/plugins/platforms/qwindows.dll" CONFIGURATIONS Release RelWithDebInfo DESTINATION "${TARGETDIR}/plugins/platforms")
	install(FILES "${QT_BASE_DIR}/plugins/platforms/qwindowsd.dll" CONFIGURATIONS Debug DESTINATION "${TARGETDIR}/plugins/platforms")
endif()
