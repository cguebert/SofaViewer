QT += widgets opengl
CONFIG += c++11

Release:TARGET = SofaFrontEndViewer
Debug:TARGET = SofaFrontEndViewer_d
TEMPLATE = app

SOURCES += main.cpp \
	core/DocumentFactory.cpp \
	core/Graph.cpp \
	core/MouseManipulator.cpp \
	core/ObjectProperties.cpp \
	core/Property.cpp \
	modules/SFELocal/Document.cpp \
	modules/SFELocal/GraphImages.cpp \
	modules/SFELocal/SofaProperties.cpp \
	render/Model.cpp \
	render/Scene.cpp \
	ui/GraphModel.cpp \
	ui/MainWindow.cpp \
	ui/OpenGLView.cpp \
	ui/PropertiesDialog.cpp \
	ui/SimpleGUIImpl.cpp \
	ui/widget/NumericalPropertyWidget.cpp \
	ui/widget/PropertyWidget.cpp \
	ui/widget/PropertyWidgetFactory.cpp \
	ui/widget/StringPropertyWidget.cpp \
	ui/widget/TablePropertyWidget.cpp

HEADERS  += \
	core/BaseDocument.h \
	core/DocumentFactory.h \
	core/Graph.h \
	core/MouseEvent.h \
	core/MouseManipulator.h \
	core/ObjectProperties.h \
	core/Property.h \
	core/SimpleGUI.h \
	core/VectorWrapper.h \
	modules/SFELocal/Document.h \
	modules/SFELocal/GraphImages.h \
	modules/SFELocal/SofaProperties.h \
	render/Model.h \
	render/Scene.h \
	ui/GraphModel.h \
	ui/MainWindow.h \
	ui/OpenGLView.h \
	ui/PropertiesDialog.h \
	ui/SimpleGUIImpl.h \
	ui/widget/PropertyWidget.h \
	ui/widget/PropertyWidgetFactory.h \
	ui/widget/SimplePropertyWidget.h \
	ui/widget/TablePropertyWidget.h

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
