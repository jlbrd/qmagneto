TEMPLATE = app
QT = gui core network xml
RC_FILE += QMagneto.rc
CONFIG += qt warn_on release
DESTDIR = ..
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
FORMS = ui/mainwindow.ui \
 ui/config.ui \
 ui/programme.ui \
 ui/programmes.ui \
 ui/about.ui
HEADERS = src/mainwindowimpl.h \
 src/xmldefaulthandler.h \
 src/graphicsrectitem.h \
 src/listmaintenant.h \
 src/visu.h
SOURCES = src/mainwindowimpl.cpp \
 src/main.cpp \
 src/xmldefaulthandler.cpp \
 src/graphicsrectitem.cpp \
 src/listmaintenant.cpp \
 src/visu.cpp
RESOURCES += ressources/ressources.qrc
INCLUDEPATH += . src src/ui
maemo {
 DEFINES +=  MAEMO
}
