#-------------------------------------------------
#
# Project created by QtCreator 2011-02-13T00:12:04
#
#-------------------------------------------------

QT       += core gui opengl xml

CONFIG += x86_64

INCLUDEPATH += /Users/oonomakoto/bass24-osx /opt/local/include /Library/Frameworks/QGLViewer.framework/Headers


DEPENDPATH += $$INCLUDEPATH

LIBS += -L/Users/oonomakoto/bass24-osx/ -lbass -lfftw3 -lbass_fx \
        -L/opt/local/lib -L/Library/Frameworks -framework QGLViewer

TARGET = Spectra
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glplotter.cpp \
    waveanlyzer.cpp \
    audiomanager.cpp

HEADERS  += mainwindow.h \
    glplotter.h \
    waveanlyzer.h \
    audiomanager.h

RESOURCES += \
    icons.qrc




