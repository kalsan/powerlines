TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    sanform/sanform_1-1.cpp \
    settings.cpp \
    player.cpp \
    powerup.cpp \
    gamescreen.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    sanform/sanform_1-1.h \
    settings.h \
    player.h \
    powerup.h \
    gamescreen.h

QMAKE_CXXFLAGS += -std=gnu++11

unix|win32: LIBS += -lSDL

unix|win32: LIBS += -lSDL_image

unix|win32: LIBS += -lSDL_ttf
