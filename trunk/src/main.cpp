#include <QApplication>
#include <QDir>
#include "mainwindowimpl.h"
//
int main(int argc, char ** argv)
{
	QApplication app( argc, argv );
#ifdef MAEMO
	QFont font = app.font();
  	font.setPointSize( font.pointSize()+6 );
  	app.setFont( font );
#endif
        // change the plugins path (add the installation directory)
        QStringList list_path ;
        QDir dir = QDir(qApp->applicationDirPath()+"/QtPlugins/");
        list_path << dir.absolutePath () << app.libraryPaths ();
        app.setLibraryPaths( list_path  );
  
	MainWindowImpl win;
	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	win.init();
	if( !win.demarrerEnIcone() )
	{
		win.showMaximized(); 
	}
	return app.exec();
}
