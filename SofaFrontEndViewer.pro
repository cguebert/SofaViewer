QT += widgets opengl
CONFIG += c++11

Release:TARGET = SofaFrontEndViewer
Debug:TARGET = SofaFrontEndViewer_d
TEMPLATE = app

SOURCES += main.cpp\
	core/Document.cpp \
	core/Graph.cpp \
	core/GraphImages.cpp \
	core/Model.cpp \
	core/MouseManipulator.cpp \
	core/ObjectProperties.cpp \
	core/Property.cpp \
	core/Scene.cpp \
	ui/GraphModel.cpp \
	ui/MainWindow.cpp \
	ui/OpenGLView.cpp \
	ui/PropertiesDialog.cpp \
	ui/PropertyWidget.cpp

HEADERS  += \
	core/Document.h \
	core/Graph.h \
	core/GraphImages.h \
	core/Model.h \
	core/MouseManipulator.h \
	core/ObjectProperties.h \
	core/Property.h \
	core/Scene.h \
	ui/GraphModel.h \
	ui/MainWindow.h \
	ui/OpenGLView.h \
	ui/PropertiesDialog.h \
	ui/PropertyWidget.h

RESOURCES     = SofaFrontEndViewer.qrc
win32:RC_FILE = SofaFrontEndViewer.rc

DESTDIR = bin

DEFINES +=	_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS

LIBRARIES="../Librairies"

INCLUDEPATH = .
INCLUDEPATH += $${LIBRARIES}/include

LIBPATH += $${LIBRARIES}/lib

# Glew
#LIBS += glew32.lib

# SFE
Release:LIBS += SofaFrontEnd_1_0rd.lib SofaFrontEndLocal_1_0rd.lib
Debug:LIBS += SofaFrontEnd_1_0d.lib SofaFrontEndLocal_1_0d.lib
