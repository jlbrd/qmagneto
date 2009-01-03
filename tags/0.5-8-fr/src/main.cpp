#include <QApplication>
#include <QDir>
#include <QDesktopWidget>
#include "mainwindowimpl.h"
//
int main(int argc, char ** argv)
{
    QApplication app( argc, argv );
    // change the plugins path (add the installation directory)
    QStringList list_path ;
    QDir dir = QDir(qApp->applicationDirPath()+"/QtPlugins/");
    list_path << dir.absolutePath () << app.libraryPaths ();
    app.setLibraryPaths( list_path  );

    MainWindowImpl win;
    app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
    QRect geo = QDesktopWidget().screenGeometry();
    geo.adjust(geo.width()/3, geo.height()/3, -geo.width()/3, -geo.height()/3);
    win.setGeometry( geo );
    win.init();
    if ( !win.demarrerEnIcone() )
    {
        win.showMaximized();
    }
    return app.exec();
}
