QT += widgets opengl
CONFIG += c++11

Release:TARGET = SofaFrontEndViewer
Debug:TARGET = SofaFrontEndViewer_d
TEMPLATE = app

SOURCES += main.cpp\
	core/Document.cpp \
	core/Model.cpp \
	core/MouseManipulator.cpp \
	core/Scene.cpp \
	ui/OpenGLView.cpp \
	ui/MainWindow.cpp

HEADERS  += \
	core/Document.h \
	core/Model.h \
	core/MouseManipulator.h \
	core/Scene.h \
	ui/OpenGLView.h \
	ui/MainWindow.h

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
