#include <QApplication>
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
	MainWindowImpl win;
	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	win.init();
	if( !win.demarrerEnIcone() )
	{
		win.showMaximized(); 
	}
	return app.exec();
}
