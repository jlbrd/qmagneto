/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2010  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qmagneto/
*
*/

#include <QApplication>
#include <QDir>
#include <QDesktopWidget>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include "mainwindowimpl.h"
#include "application.h"
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
int main(int argc, char ** argv)
{
    QApplication app( argc, argv );
    QStringList list_path ;
    QDir dir = QDir(app.applicationDirPath()+"/QtPlugins/");
    list_path << dir.absolutePath () << app.libraryPaths ();
    app.setLibraryPaths( list_path  );
	//
	//
	QTranslator translatorQMagneto, translatorQt;
	QString language = QLocale::languageToString( QLocale::system().language() );
	//
/*	for(int i=0; i<QString(app.argv()[ 1 ]).split(" ",QString::SkipEmptyParts).count(); i++)
	{
		QString s = QString(app.argv()[ 1 ]).split(" ",QString::SkipEmptyParts).at(i);
		if( s == "-l" )
		{
			language = QString(app.argv()[ 2 ]).split(" ",QString::SkipEmptyParts).at(i);
		}
    }*/
	app.processEvents();
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
    //app.connect(&app, SIGNAL(messageReceived(const QString&)), &win, SLOT(show()));
    app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
    QRect geo = QDesktopWidget().screenGeometry();
    geo.adjust(geo.width()/3, geo.height()/3, -geo.width()/3, -geo.height()/3);
    win.setGeometry( geo );
    win.init();
    if ( !win.systrayStarts() )
    {
        win.show();
    }
    win.slotScheduledUpdate();

    return app.exec();
}
