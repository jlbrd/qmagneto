TEMPLATE = app
QT = gui \
 core \
 network \
 xml \
 sql
RC_FILE += qmagneto.rc
CONFIG += qt warn_on release
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
 ui/about.ui \
 ui/channels.ui \
 ui/newversion.ui \
 ui/findwidget.ui \
 ui/modifyprogram.ui \
 ui/findglobal.ui \
 ui/changeicon.ui \
 ui/editurl.ui \
 ui/changethumb.ui
HEADERS += src/mainwindowimpl.h \
 src/xmldefaulthandler.h \
 src/graphicsrectitem.h \
 src/listnow.h \
 src/channelsimpl.h \
 src/programimpl.h \
 src/configimpl.h \
 src/releaseversion.h \
 src/defs.h \
 src/modifyprogramimpl.h \
 src/googleimage.h \
 src/findglobalimpl.h \
 src/downloadmanager.h \
 src/getlastversion.h \
 src/channeliconitem.h \
 src/changeiconimpl.h \
 src/changethumbimpl.h
SOURCES += src/mainwindowimpl.cpp \
 src/main.cpp \
 src/xmldefaulthandler.cpp \
 src/graphicsrectitem.cpp \
 src/listnow.cpp \
 src/channelsimpl.cpp \
 src/programimpl.cpp \
 src/configimpl.cpp \
 src/modifyprogramimpl.cpp \
 src/googleimage.cpp \
 src/findglobalimpl.cpp \
 src/downloadmanager.cpp \
 src/getlastversion.cpp \
 src/channeliconitem.cpp \
 src/changeiconimpl.cpp \
 src/changethumbimpl.cpp
RESOURCES += resources/resources.qrc
INCLUDEPATH += . src
maemo {
 DEFINES +=  MAEMO
}
TRANSLATIONS += resources/translations/French.ts
