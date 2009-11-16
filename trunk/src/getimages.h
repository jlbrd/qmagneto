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

#ifndef RECUPIMAGES_H
#define RECUPIMAGES_H
//
#include <QSqlQuery>
#include <QHttp>
#include <QPixmap>
#include "xmldefaulthandler.h"
#include "googleimage.h"

//
class GetImages : public QObject
{
Q_OBJECT
public:
	PairIcon pairIcon(QString icon, QSqlQuery query);
	void setList(QList<Pair> list=QList<Pair>(), QSqlQuery query=QSqlQuery(), QString proxyAddress=QString(), int proxyPort=0, QString proxyUsername=QString(), QString proxyPassword=QString());
	void imageToTmp(QString icon, QSqlQuery query, bool isChannel);
	void get();
	GetImages();
	~GetImages();
private:
	//QStringList m_list;
    QList<Pair> m_pairList;
	QSqlQuery m_query;
	QHttp *m_http;
private slots:
	void slotRequestFinished(bool err);
signals:
	void imageAvailable(PairIcon);
};
#endif
