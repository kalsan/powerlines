TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    sanform/sanform_1-1.cpp \
    settings.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    sanform/sanform_1-1.h \
    settings.h

QMAKE_CXXFLAGS += -std=gnu++11

unix|win32: LIBS += -lSDL

unix|win32: LIBS += -lSDL_image

unix|win32: LIBS += -lSDL_ttf
