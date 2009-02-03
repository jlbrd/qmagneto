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

#ifndef __DEFS_H__
#define __DEFS_H__

#include <QList>
#include <QDateTime>
#include <QMetaType>

typedef struct
{
	QString id;
	QString name;
	QString icon;	
	bool enabled;
} TvChannel;
Q_DECLARE_METATYPE(TvChannel)
//
class PairIcon
{
public:
	PairIcon(QString s=QString(), QPixmap p=QPixmap());
	QString icon() { return m_icon; }
	QPixmap pixmap() { return m_pixmap; }
private:
	QString m_icon;
	QPixmap m_pixmap;
};
Q_DECLARE_METATYPE(PairIcon)
//
typedef struct
{
	QDateTime start;
	QDateTime stop;
	QString channel;
	QString channelName;
	QString title;
	QString subTitle;
	QStringList category;
	QStringList resume;
	QString story;
	QString aspect;
	QString credits;
	QString director;
	QString star;
	QString icon;
} TvProgram;
Q_DECLARE_METATYPE(TvProgram)

#endif // __DEFS_H__
