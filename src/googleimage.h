/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2009  Jean-Luc Biord
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
* Program URL   : http://code.google.com/p/qmagneto/
*
*/
#ifndef GOOGLEIMAGE_H
#define GOOGLEIMAGE_H
//
#include <QObject>
#include <QHttp>
#include <QSqlQuery>
#include "defs.h"
//
typedef QPair<QString, QString> Pair;
class GetImages;
class MainWindowImpl;

class GoogleImage : public QObject
{
Q_OBJECT
public:
	void stop();
	GoogleImage(MainWindowImpl *parent=0);
	void setList(QStringList list, QSqlQuery query, QString proxyAddress=QString(), int proxyPort=0, QString proxyUsername=QString(), QString proxyPassword=QString());
	~GoogleImage();
	void imageToTmp(QString icon, QSqlQuery query, bool isChannel);
	PairIcon pairIcon(QString icon, QSqlQuery query);
private:	
	void google_search(QString s);
	void getThumbnail(QString URL);
    QString parse_html();
    QString *html;
    QHttp *httpURL;
    QString *resultHtml;
    QRegExp rx_href;
    QRegExp rx_data,rx_start,rx_other;
    MainWindowImpl *m_main;
    QStringList m_list;
    Pair m_pair;
    QSqlQuery m_query;
    QString m_proxyAddress;
    int m_proxyPort;
    QString m_proxyUsername;
    QString m_proxyPassword;
	QHttp *m_httpThumbnail;
private slots:
	void httpURL_done ( bool err );
	void httpThumbnail_done(bool err);
signals:
	void imageAvailable(PairIcon);
};
#endif
