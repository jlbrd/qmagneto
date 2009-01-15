TEMPLATE = app
QT = gui \
 core \
 network \
 xml \
 sql
RC_FILE += QMagneto.rc
CONFIG += qt warn_on console release
DESTDIR = bin
unix {
 OBJECTS_DIR +=  build/o/unix
}
win32 {
 OBJECTS_DIR +=  build/o/win32
 CONFIG -=  debug_and_release
}
macx {
 ICON +=  resources/images/tv.icns
 OBJECTS_DIR +=  build/o/mac
}
MOC_DIR = build
UI_DIR = build
FORMS = ui/mainwindow.ui \
 ui/config.ui \
 ui/program.ui \
 ui/programs.ui \
 ui/about.ui \
 ui/channels.ui \
 ui/newversion.ui \
 ui/findwidget.ui
HEADERS = src/mainwindowimpl.h \
 src/xmldefaulthandler.h \
 src/graphicsrectitem.h \
 src/listnow.h \
 src/channelsimpl.h \
 src/getimages.h \
 src/programimpl.h \
 src/configimpl.h \
 src/releaseversion.h \
 src/defs.h
SOURCES = src/mainwindowimpl.cpp \
 src/main.cpp \
 src/xmldefaulthandler.cpp \
 src/graphicsrectitem.cpp \
 src/listnow.cpp \
 src/channelsimpl.cpp \
 src/getimages.cpp \
 src/programimpl.cpp \
 src/configimpl.cpp
RESOURCES += resources/resources.qrc
INCLUDEPATH += . src src/ui
maemo {
 DEFINES +=  MAEMO
}
TRANSLATIONS += resources/translations/French.ts
