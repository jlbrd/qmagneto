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

#ifndef XMLDEFAULTHANDLER_H
#define XMLDEFAULTHANDLER_H
//
#include <QXmlDefaultHandler>
#include <QSqlQuery>
#include <QList>
#include <QDateTime>
#include <QMetaType>
#include <QFont>
#include <QPixmap>

#include "graphicsrectitem.h"
//#include "getimages.h"
#include "defs.h"
#include "googleimage.h"

class QGraphicsView;
class MainWindowImpl;
//class GetImages;
//
class XmlDefaultHandler : public QXmlDefaultHandler
{
private:
	QStringList m_categories;
    QList<TvProgram> m_eveningPrograms;
	bool m_running;
	bool m_stop;
	bool connectNewDB();
	QStringList m_sortedChannelsList;
	int m_nbEntries;
	int m_nbEntry;
	int m_programId;
    float m_hourHeight;
    float m_progHeight;
    float m_progWidth;
    void clearView();
    void listProgrammesSortedByTime();
    QList<TvProgram> sortedPrograms(QList<TvProgram> list);
    QList<TvChannel> sortedChannels();
    QDate m_date;
    int m_hourBeginning;
    bool endDocument();
    enum Balise { None, Channel, Title, SubTitle, Desc, Category, Aspect, DisplayName, Director, Actors, Date, Star};
    Balise m_balise;
    MainWindowImpl *m_main;
    QGraphicsView *m_programsView;
    TvChannel m_chaineTV;
    QList<TvChannel> m_TvChannelsList;
    TvProgram m_programTV;
    QList<int> m_TvProgramsList;
    QList<GraphicsRectItem *> m_listeItemChaines;
    QList<GraphicsRectItem *> m_listeItemHeures;
    QList<GraphicsRectItem *> m_programsItemsList;
    QList<GraphicsRectItem *> m_programsSortedItemsList;
    QGraphicsLineItem *m_currentTimeLine;
    QString m_ch;
    QSqlQuery m_query;
    QSqlQuery m_queryNewBase;
    bool connectDB();
    GoogleImage *m_googleImage;
protected:
    virtual bool startDocument();
public:
	QStringList categories(bool forceReading=false);
	void setPositionOnChannelMode(QGraphicsView *view);
	void expandItem(GraphicsRectItem *item, bool expand);
	QStringList readChannelFromDB(QString channelName);
	QSqlQuery query(QString s);
	bool writeThumbnailInDB(QVariant clob, QString title);
	QList<TvChannel> disabledChannels();
	void setEnableChannel(QString name, bool enabled);
	bool running() { return m_running; }
	bool stopped() { return m_stop; }
	void setStop(bool value);
	void setSortedChannelsList();
	QStringList getSortedChannelsList();
	TvProgram tvProgram(int id);
	void setNbEntries(int value) { m_nbEntries = value; }
	static QString channelIconName(QString name);
	bool programOutdated(int day);
    GraphicsRectItem * findProgramme(QString text, bool backward, bool fromBegin, bool sensitive, bool wholeWord);
    //GetImages *getImages() 
    //{
    	//return m_getImages;
   	//}
    //GoogleImage *googleImage() 
    //{
    	//return m_googleImage;
   	//}
    void setHourHeight(float value)
    {
        m_hourHeight = value;
    }
    float hourHeight()
    {
        return m_hourHeight;
    }
    void setProgHeight(float value)
    {
        m_progHeight = value;
    }
    float progHeight()
    {
        return m_progHeight;
    }
    void setProgWidth(float value)
    {
        m_progWidth = value;
    }
    float progWidth()
    {
        return m_progWidth;
    }
    QDate maximumDate();
    QDate minimumDate();
    void nowCenter();
    PairIcon pairIcon(QString title);
    void imageToTmp(QString title, bool isChannel);
    QList<TvChannel> channels()
    {
        return m_TvChannelsList;
    }
    //QList<TvProgram>  programsMaintenant();
    //QList<TvProgram> eveningPrograms(bool read=false);
    void evening();
    void setHeureDebutJournee( int value)
    {
        m_hourBeginning = value;
    }
    void currentTimeLinePosition(QList<int> idList, bool hidden);
    void init();
    void setDate(QDate value);
    void deplaceHeures(int value);
    void deplaceChaines(int value);
    bool characters( const QString & ch );
    bool endElement( const QString & namespaceURI, const QString & localName, const QString & qName );
    bool startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
    XmlDefaultHandler(MainWindowImpl *main, QGraphicsView *programs);
    ~XmlDefaultHandler();
    QList<GraphicsRectItem *> listItemProgrammes()
    {
        return m_programsItemsList;
    };
    //QList<GraphicsRectItem *> listSortedItemProgrammes() { return m_programsSortedItemsList; };
    QStringList readProgrammesFromDB();
	void readThumbsFromDB(QStringList list);
};
#endif
