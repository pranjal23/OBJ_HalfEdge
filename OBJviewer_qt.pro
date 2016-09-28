
QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OBJViewerQt
TEMPLATE = app

win32 {
    LIBS+=-lopengl32
    LIBS+=-lglu32
}

QMAKE_MAC_SDK = macosx10.12

SOURCES += main.cpp\
        window.cpp \
    trianglemesh.cpp \
    mfileparser.cpp \
    viewportwidget.cpp \
    parseworker.cpp

HEADERS  += window.h \
    trianglemesh.h \
    mfileparser.h \
    viewportwidget.h \
    parseworker.h

FORMS    += window.ui

RESOURCES += \
    OBJviewer_resources.qrc
