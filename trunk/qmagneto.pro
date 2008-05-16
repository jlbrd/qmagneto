TEMPLATE = app
QT = gui \
 core \
 network \
 xml \
 sql
RC_FILE += QMagneto.rc
CONFIG += qt warn_on release 
DESTDIR = ..
unix {
 OBJECTS_DIR +=  build/o/unix
}
win32 {
 OBJECTS_DIR +=  build/o/win32
 CONFIG -=  debug_and_release
}
MOC_DIR = build
UI_DIR = build
FORMS = ui/mainwindow.ui \
 ui/config.ui \
 ui/programme.ui \
 ui/programmes.ui \
 ui/about.ui \
 ui/canaux.ui
HEADERS = src/mainwindowimpl.h \
 src/xmldefaulthandler.h \
 src/graphicsrectitem.h \
 src/listmaintenant.h \
 src/visu.h \
 src/canauximpl.h
SOURCES = src/mainwindowimpl.cpp \
 src/main.cpp \
 src/xmldefaulthandler.cpp \
 src/graphicsrectitem.cpp \
 src/listmaintenant.cpp \
 src/visu.cpp \
 src/canauximpl.cpp
RESOURCES += ressources/ressources.qrc
INCLUDEPATH += . src src/ui
maemo {
 DEFINES +=  MAEMO
}
