cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "SofaFrontEndViewer")
project(${PROJECT_NAME})

set(QT_BASE_DIR "" CACHE PATH "Qt5 install prefix") # C:/Qt/5/msvc2015/
list(APPEND CMAKE_PREFIX_PATH ${QT_BASE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(HEADER_FILES
	GraphModel.h
	GraphView.h
	MainWindow.h
	OpenGLView.h
	PropertiesDialog.h
	simplegui/ButtonImpl.h
	simplegui/DialogImpl.h
	simplegui/MenuImpl.h
	simplegui/PanelImpl.h
	simplegui/SettingsImpl.h
	simplegui/SimpleGUIImpl.h
	widget/PropertyWidget.h
	widget/PropertyWidgetFactory.h
	widget/SimplePropertyWidget.h
	widget/TablePropertyWidget.h
)

set(SOURCE_FILES
	main.cpp
	GraphModel.cpp
	GraphView.cpp
	MainWindow.cpp
	OpenGLView.cpp
	PropertiesDialog.cpp
	simplegui/ButtonImpl.cpp
	simplegui/DialogImpl.cpp
	simplegui/MenuImpl.cpp
	simplegui/PanelImpl.cpp
	simplegui/SettingsImpl.cpp
	simplegui/SimpleGUIImpl.cpp
	widget/ListPropertyWidget.cpp
	widget/NumericalPropertyWidget.cpp
	widget/PropertyWidget.cpp
	widget/PropertyWidgetFactory.cpp
	widget/StringPropertyWidget.cpp
	widget/TablePropertyWidget.cpp
)

GroupFiles(HEADER_FILES)
GroupFiles(SOURCE_FILES)

set(RESSOURCE_FILES ../${PROJECT_NAME}.qrc)
if(WIN32)
	set(RESSOURCE_FILES ${RESSOURCE_FILES} ../${PROJECT_NAME}.rc)
endif()

find_package(Qt5Widgets)
find_package(Qt5OpenGL)

if(WIN32)
	set(GUI_TYPE WIN32)
endif()

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${RESSOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Applications")

target_link_libraries(${PROJECT_NAME} "Core")
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${TARGET_DIR})

# Install Qt files
if(WIN32)
	set(QT_LIBS Qt5Core Qt5Gui Qt5OpenGL Qt5Widgets)
	
	set(qtLibsRelease)
	set(qtLibsDebug)
	
	foreach(libName ${QT_LIBS})
		list(APPEND qtLibsRelease	"${QT_BASE_DIR}/bin/${libName}.dll")
		list(APPEND qtLibsDebug		"${QT_BASE_DIR}/bin/${libName}d.dll")
	endforeach(libName)
	
	install(FILES ${qtLibsRelease} CONFIGURATIONS Release RelWithDebInfo DESTINATION ${TARGET_DIR})
	install(FILES ${qtLibsDebug} CONFIGURATIONS Debug DESTINATION ${TARGET_DIR})
	
	install(FILES "${QT_BASE_DIR}/plugins/platforms/qwindows.dll" CONFIGURATIONS Release RelWithDebInfo DESTINATION "${TARGET_DIR}/platforms")
	install(FILES "${QT_BASE_DIR}/plugins/platforms/qwindowsd.dll" CONFIGURATIONS Debug DESTINATION "${TARGET_DIR}/platforms")
endif()
