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

#ifndef RECUPIMAGES_H
#define RECUPIMAGES_H
//
#include <QSqlQuery>
#include <QHttp>
#include <QPixmap>
#include "xmldefaulthandler.h"

//
class GetImages : public QObject
{
Q_OBJECT
public:
	PairIcon pairIcon(QString icon, QSqlQuery query);
	void setList(QStringList list, QSqlQuery query, QString proxyAddress=QString(), int proxyPort=0);
	void imageToTmp(QString icon, QSqlQuery query, bool isChannel);
	void get();
	GetImages(QStringList list, QSqlQuery query);
	~GetImages();
private:
	QStringList m_list;
	QSqlQuery m_query;
	QHttp *m_http;
private slots:
	void slotRequestFinished(bool err);
signals:
	void imageAvailable(PairIcon);
};
#endif
