/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2009  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify.
*  But to reuse the source code, permission of the author is essential. Without authorization, code reuse is prohibited.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://code.google.com/p/qmagneto/
*
*/


#include <QApplication>
#include <QDir>
#include <QDesktopWidget>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
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
	//
	//
	QTranslator translatorQMagneto, translatorQt;
	QString language = QLocale::languageToString( QLocale::system().language() );
	//
	for(int i=0; i<QString(app.argv()[ 1 ]).split(" ",QString::SkipEmptyParts).count(); i++)
	{
		QString s = QString(app.argv()[ 1 ]).split(" ",QString::SkipEmptyParts).at(i);
		if( s == "-l" )
		{
			language = QString(app.argv()[ 2 ]).split(" ",QString::SkipEmptyParts).at(i);
		}
	}
	qApp->processEvents();
	//
	// load & install translation
	translatorQMagneto.load( ":/translations/translations/"+language+".qm" );
	app.installTranslator( &translatorQMagneto );
	// search, load & install Qt translation
	translatorQt.load( ":/translations/translations/Qt_"+language+".qm" );
	if (translatorQt.isEmpty())
		translatorQt.load( QLibraryInfo::location( QLibraryInfo::TranslationsPath) + "/qt_"+QLocale::system().name()+".qm" );
	if (!translatorQt.isEmpty())
		app.installTranslator( &translatorQt );
	//
    MainWindowImpl win;
    app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
    QRect geo = QDesktopWidget().screenGeometry();
    geo.adjust(geo.width()/3, geo.height()/3, -geo.width()/3, -geo.height()/3);
    win.setGeometry( geo );
    win.init();
    if ( !win.systrayStarts() )
    {
        win.showMaximized();
    }
    return app.exec();
}
