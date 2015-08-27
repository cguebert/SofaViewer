cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SofaFrontEndViewer")
project(${PROJECT_NAME})

set(LIBRARIES_BASE_DIR "" CACHE PATH "Directory where other libraries are installed")

include_directories(".")

# Librairies
add_subdirectory("render")

# Plugins
add_subdirectory("modules/SFELocal")

set(QT_BASE_DIR "" CACHE PATH "Qt5 install prefix") # C:/Qt/5/msvc2015/
list(APPEND CMAKE_PREFIX_PATH ${QT_BASE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(HEADER_FILES
	core/BaseDocument.h
	core/DocumentFactory.h
	core/Graph.h
	core/MouseEvent.h
	core/MouseManipulator.h
	core/ObjectProperties.h
	core/Property.h
	core/SimpleGUI.h
	core/VectorWrapper.h
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
	core/MouseManipulator.cpp
	core/ObjectProperties.cpp
	core/Property.cpp
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
