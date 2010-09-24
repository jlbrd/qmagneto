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

#include "xmldefaulthandler.h"
#include "graphicsrectitem.h"
#include "mainwindowimpl.h"
#include "findglobalimpl.h"
#include "expandedpixmap.h"
//
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QGraphicsScene>
#include <QSettings>
#include <QGraphicsView>
#include <QScrollBar>
#include <QTextCodec>
#include <QObject>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
QGraphicsView *viewP;
PairIcon::PairIcon(QString s, QPixmap p)
{
    m_icon = s;
    m_pixmap = p;
}

XmlDefaultHandler::XmlDefaultHandler(MainWindowImpl *main, QGraphicsView *programs)
        : QXmlDefaultHandler(), m_main(main), m_programsView(programs)
{
    //m_getImages = new GetImages();
    m_running = false;
    m_stop = false;
    m_googleImage = new GoogleImage(m_main, this);
    viewP = programs;
}
//
XmlDefaultHandler::~XmlDefaultHandler()
{
    if ( m_googleImage )
    {
        m_googleImage->stop();
        m_googleImage->deleteLater();
    }
}
void  XmlDefaultHandler::setStop(bool value)
{
    m_googleImage->stop();
    m_stop = value;
}
//
bool XmlDefaultHandler::startElement( const QString & , const QString & , const QString & qName, const QXmlAttributes & atts )
{
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    static bool isChannel = false;
    QString dateTimePattern = "yyyyMMddhhmmss";
    Balise balisePrecedente = m_balise;
    m_balise = None;
    if ( qName == "channel" )
    {
        isChannel = true;
        m_balise = Channel;
        for (int i=0; i< atts.count(); i++)
        {
            if ( atts.qName(i) == "id" )
            {
                m_chaineTV.id = atts.value(i);
            }
        }
    }
    else if ( qName == "display-name" )
    {
        m_balise = DisplayName;
    }
    else if ( qName == "title" )
    {
        m_balise = Title;
    }
    else if ( qName == "icon" )
    {
        if ( isChannel )
            m_chaineTV.icon = atts.value(0);
        else
            m_programTV.icon = atts.value(0);
        if ( !isChannel )
            QD<<atts.value(0);
        //m_imagesList.append( atts.value(0) );
    }
    else if ( qName == "sub-title" )
    {
        m_balise = SubTitle;
    }
    else if ( qName == "desc" )
    {
        m_balise = Desc;
    }
    else if ( qName == "aspect" )
    {
        m_balise = Aspect;
    }
    else if ( qName == "category" )
    {
        m_balise = Category;
    }
    else if ( qName == "director" )
    {
        m_balise = Director;
    }
    else if ( qName == "actor" )
    {
        m_balise = Actors;
    }
    else if ( qName == "date" )
    {
        m_balise = Date;
    }
    else if ( qName == "star-rating" )
    {
        m_balise = Star;
    }
    else if ( qName == "value" )
    {
        if ( balisePrecedente == Star )
            m_balise = Star;
        else
            m_balise = None;
    }
    else if ( qName == "programme" )
    {
        isChannel = false;
        for (int i=0; i< atts.count(); i++)
        {
            if ( atts.qName(i) == "start" )
            {
                QString dateTime = atts.value(i).section(" ",0,0);
                m_programTV.start = QDateTime::fromString(dateTime, dateTimePattern.left(dateTime.length()));
            }
            else if ( atts.qName(i) == "stop" )
            {
                QString dateTime = atts.value(i).section(" ",0,0);
                m_programTV.stop = QDateTime::fromString(dateTime, dateTimePattern.left(dateTime.length()));
            }
            else if ( atts.qName(i) == "channel" )
            {
                m_programTV.channel = atts.value(i);
            }
        }
    }
    else
    {
        m_balise = None;
    }
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    return true;
}

bool XmlDefaultHandler::endElement( const QString & , const QString & , const QString & qName )
{
    static unsigned int id = 0;
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    if ( qName == "channel" )
    {
        if ( (Qt::CheckState)settings.value(m_chaineTV.id+"-isEnabled", Qt::Checked).toInt() != Qt::Checked )
        {
            QD << QObject::tr("Channel %1 disabled in Channels dialog").arg(m_chaineTV.name);
            m_chaineTV.enabled = false;
        }
        else
            m_chaineTV.enabled = true;
        QString queryString = "insert into channels values(";
        queryString = queryString
                      + "'" + m_chaineTV.id.replace("'", "$") + "', "
                      + "'" + m_chaineTV.name.replace("'", "$") + "', "
                      + "'" + m_chaineTV.icon.replace("'", "$") + "', "
                      + "'" + QString::number(m_chaineTV.enabled).replace("'", "$") + "')";
        bool rc = m_queryNewBase.exec(queryString);
        if (rc == false)
        {
            QD << "Failed to insert record to db" << m_queryNewBase.lastError();
            QD << queryString;
        }
        m_TvChannelsList << m_chaineTV;
        m_chaineTV = TvChannel();
    }
    else if ( qName == "programme" )
    {
        m_main->slotDataReadProgress(m_nbEntry++, m_nbEntries);
        if ( (Qt::CheckState)settings.value(m_programTV.channel+"-isEnabled", Qt::Checked).toInt() != Qt::Checked )
        {
            return true;
        }
        foreach(TvChannel channel, m_TvChannelsList)
        {
            if ( channel.id == m_programTV.channel )
            {
                m_programTV.channelName = channel.name;
                break;
            }
        }
        m_queryNewBase.prepare(
            "insert into programs (id, start, stop, channel, channelName, title, subTitle, category, "
            "resume, story, aspect, credits, director, actors, date, star, icon)"
            " values ("
            ":id, :start, :stop, :channel, :channelName, :title, :subTitle, :category, "
            ":resume, :story, :aspect, :credits, :director, :actors, :date, :star, :icon"
            ")"
        );
        //QString id = QString::number(m_programTV.start.date().day())
        //+ m_programTV.start.time().toString("hhmm")
        //+ m_programTV.channel.section('C', 1).section('.', 0, 0);
        m_queryNewBase.bindValue(":id", id++);
        m_queryNewBase.bindValue(":start", QString::number( m_programTV.start.toTime_t() ));
        m_queryNewBase.bindValue(":stop", QString::number( m_programTV.stop.toTime_t() ));
        m_queryNewBase.bindValue(":channel", QString(m_programTV.channel).replace("'", "$"));
        m_queryNewBase.bindValue(":channelName", QString(m_programTV.channelName).replace("'", "$"));
        m_queryNewBase.bindValue(":title", QString(m_programTV.title).replace("'", "$"));
        m_queryNewBase.bindValue(":subTitle",  QString(m_programTV.subTitle).replace("'", "$"));
        m_queryNewBase.bindValue(":category", QString(m_programTV.category.join("|")).replace("'", "$"));
        m_queryNewBase.bindValue(":resume", QString(m_programTV.resume.join("|")).replace("'", "$").toUtf8() );//qcompress
        m_queryNewBase.bindValue(":story", QString(m_programTV.story).replace("'", "$").toUtf8() );
        m_queryNewBase.bindValue(":aspect", QString(m_programTV.aspect).replace("'", "$"));
        m_queryNewBase.bindValue(":credits", QString(m_programTV.credits).replace("'", "$"));
        m_queryNewBase.bindValue(":director", QString(m_programTV.director).replace("'", "$"));
        m_queryNewBase.bindValue(":actors", QString(m_programTV.actors.join("|")).replace("'", "$"));
        m_queryNewBase.bindValue(":date", QString(m_programTV.date).replace("'", "$"));
        m_queryNewBase.bindValue(":star", QString(m_programTV.star).replace("'", "$"));
        m_queryNewBase.bindValue(":icon",  QString(m_programTV.icon).replace("'", "$"));

        bool rc = m_queryNewBase.exec();
        if (rc == false)
        {
            QD << "Failed to insert record to db" << m_queryNewBase.lastError();
            //QD << queryString;
        }
        bool containsCategory = false;
        if ( m_main->groupGoogleImageCategories() )
        {
            foreach(QString categ, m_main->googleImageCategories())
            {
                foreach(QString category, m_programTV.category)
                {
                    if ( category.toLower().simplified() == categ.toLower().simplified()  )
                    {
                        containsCategory = true;
                        break;
                    }
                }
            }
        }
        else
            containsCategory = true;
        if ( !m_programTV.title.isEmpty() && containsCategory )
        {
            QString googleTitle;
            //googleTitle += " \"" + channelName + "\" ";
            googleTitle += " \"" + m_programTV.title + "\" " ;
            if ( !m_programTV.subTitle.isEmpty() )
                googleTitle += " \"" + m_programTV.subTitle + "\" ";
            if ( !m_programTV.director.isEmpty() )
                googleTitle += " \"" + m_programTV.director + "\" ";
            bool rc = m_queryNewBase.exec("select dayOrder from images where title='" + QString(googleTitle).replace("'", "$").replace("\"", "µ") + "'");
            if ( !m_queryNewBase.next() )
            {
                QByteArray data;
                QVariant clob(data);
                m_queryNewBase.prepare("INSERT INTO images (title, url, ok, data, dayOrder)"
                                       "VALUES (:title, :url, :ok, :data, :dayOrder)");
                m_queryNewBase.bindValue(":title", QString(googleTitle).replace("'", "$").replace("\"", "µ"));
                m_queryNewBase.bindValue(":url", QString(m_programTV.icon).replace("'", "$").replace("\"", "µ"));
                m_queryNewBase.bindValue(":ok","0");
                m_queryNewBase.bindValue(":data", clob);
                int dayOrder = abs(QDateTime::currentDateTime().secsTo(m_programTV.start));
                m_queryNewBase.bindValue(":dayOrder", dayOrder);
                bool rc = m_queryNewBase.exec();
                if (rc == false)
                {
                    QD << "Failed to insert record to db" << m_queryNewBase.lastError();
                    //QD << queryString;
                }
            }
            else
            {
                int oldDayOrder = m_queryNewBase.value(0).toInt();
                int dayOrder = abs(QDateTime::currentDateTime().secsTo(m_programTV.start));
                if ( dayOrder < oldDayOrder )
                {
                    m_queryNewBase.prepare("update images set dayOrder=:dayOrder where title='" +QString(googleTitle).replace("'", "$").replace("\"", "µ")+ "'");
                    m_queryNewBase.bindValue(":dayOrder", dayOrder);
                    bool rc = m_queryNewBase.exec();
                    if (rc == false)
                    {
                        QD << "Failed to insert record to db" << m_queryNewBase.lastError();
                    }
                }
            }
        }
        m_programTV = TvProgram();
    }
    settings.endGroup();
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    return true;
}


bool XmlDefaultHandler::characters( const QString & ch )
{
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    if ( ch.simplified().isEmpty() )
        return true;
    switch ( m_balise )
    {
    case Title:
        m_programTV.title = ch;
        break;
    case SubTitle:
        m_programTV.subTitle = ch;
        break;
    case Desc:
        if ( !m_programTV.resume.count() )
            m_programTV.resume << ch;
        else
            m_programTV.story = ch;
        break;
    case Aspect:
        m_programTV.aspect = ch;
        break;
    case Category:
        m_programTV.category << ch;
        break;
    case DisplayName:
        if ( m_chaineTV.name.isEmpty() )
            m_chaineTV.name = ch;
        else
            m_chaineTV.name += QChar(255) + ch;
        break;
    case Director:
        m_programTV.director = ch;
        break;
    case Actors:
        m_programTV.actors << ch;
        break;
    case Date:
        m_programTV.date = ch;
        break;
    case Star:
        m_programTV.star = ch;
        break;
    default:
        m_ch = ch;
        break;
    }
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    return true;
}


bool XmlDefaultHandler::endDocument()
{
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    QString queryString ="insert into channels(id, name, icon, enabled) "
                         +QString("select distinct channel, channel, '', 1 ")
                         +"from programs "
                         +"where not exists ( select 1 from channels where id=programs.channel) ";

    bool rc = m_queryNewBase.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to insert record to db" << m_queryNewBase.lastError();
        QD << queryString;
    }

    m_queryNewBase.exec("END TRANSACTION;");

    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    if ( settings.value( "pos0" ).toString().isEmpty() )
    {
        int pos = 0;
        foreach(TvChannel channel, m_TvChannelsList)
        {
            settings.setValue(channel.id+"-isEnabled", Qt::Checked);
            settings.setValue("pos"+QString::number(pos++), channel.id);
        }
        setSortedChannelsList();
    }
    settings.endGroup();

    QString db = m_main->iniPath() + "qmagnetoa.db";
    QSqlDatabase::database(db).close();
    QSqlDatabase::removeDatabase(db);
    QD<<QSqlDatabase::database(db).isOpen();

    db = m_main->iniPath() + "qmagnetob.db";
    QSqlDatabase::database(db).close();
    QSqlDatabase::removeDatabase(db);
    QD<<QSqlDatabase::database(db).isOpen();

    QString name = m_main->databaseName();
    if ( name == "qmagnetoa.db" )
    {
        name = "qmagnetob.db";
    }
    else
    {
        name = "qmagnetoa.db";
    }
    m_main->setDatabaseName(name);
    m_running = false;
    if ( m_stop )
    {
        QD;
        m_running = false;
        return false;
    }
    return true;
}

void XmlDefaultHandler::deplaceChaines(int )
{
    foreach(GraphicsRectItem *item, m_listeItemChaines)
    {
        item->setPos(m_programsView->horizontalScrollBar()->value(), item->y());
    }
    //m_programsView->update();
}


void XmlDefaultHandler::deplaceHeures(int )
{
    foreach(GraphicsRectItem *item, m_listeItemHeures)
    {
        item->setPos(item->x(), m_programsView->verticalScrollBar()->value());
    }
    //m_programsView->update();
}


bool XmlDefaultHandler::startDocument()
{
    m_running = true;
    QString queryString;
    m_programId = 0;
    m_googleImage->stop();
    //delete m_googleImage;
    //m_googleImage->deleteLater();
    //m_googleImage = new GoogleImage(m_main, this);

    connectNewDB();
    bool rc = m_queryNewBase.exec(QLatin1String("PRAGMA synchronous=OFF"));
    if (rc == false)
    {
        QD << "Failed to insert record to db" << m_queryNewBase.lastError();
        QD << "PRAGMA synchronous=OFF";
    }
    m_nbEntry = 0;
    //m_categories.clear();
    m_main->setProgressBarFormat(QObject::tr("Parsing %p%"));
    queryString = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
    m_queryNewBase.exec(queryString);
    QStringList listTables;
    while ( m_queryNewBase.next() )
    {
        listTables << m_queryNewBase.value(0).toString();
    }
    foreach(QString table, listTables)
    {
        queryString =  "drop table " + table + ";";
        bool rc = m_queryNewBase.exec(queryString);
        if (rc == false)
        {
            QD << "Failed to delete table" << m_queryNewBase.lastError();
            QD << queryString;
        }
    }
    queryString = "create table channels ("
                  "id string,"
                  "name string,"
                  "icon string,"
                  "enabled string"
                  ")";

    m_queryNewBase.exec(queryString);
    //
    queryString = "create table programs ("
                  "id int,"
                  "start int,"
                  "stop int,"
                  "channel string,"
                  "channelName string,"
                  "title string,"
                  "subTitle string,"
                  "category string,"
                  "resume blob,"
                  "story blob,"
                  "aspect string,"
                  "credits string,"
                  "director string,"
                  "actors string,"
                  "date string,"
                  "star string,"
                  "icon string"
                  ")";
    //
    m_queryNewBase.exec(queryString);
    //queryString = "create index programsindex on programs (start, stop)";
    //rc = m_queryNewBase.exec(queryString);
    //if (rc == false)
    //{
    //QD << "Failed to delete table" << m_queryNewBase.lastError();
    //QD << queryString;
    //}
    queryString = "create table images ("
                  "title string,"
                  "url string,"
                  "ok string,"
                  "data blob,"
                  "dayOrder int"
                  ")";

    m_queryNewBase.exec(queryString);
    m_queryNewBase.exec("BEGIN TRANSACTION;");
    return true;
}


void XmlDefaultHandler::init()
{
    m_TvChannelsList.clear();
    m_TvProgramsList.clear();
    m_programsItemsList.clear();
    //m_imagesList.clear();
}


void XmlDefaultHandler::currentTimeLinePosition(QList<int> idList, bool hidden)
{
    double x = QTime(0,0).secsTo( QTime::currentTime() )*(m_progWidth/1800.0);
    if ( ! hidden )
    {
        m_currentTimeLine->setLine
        (
            100+x-((m_hourBeginning*2)*m_progWidth),
            m_hourHeight,
            100+x-((m_hourBeginning*2)*m_progWidth) ,
            m_hourHeight+(m_TvChannelsList.count()*m_progHeight)
        );
    }
    foreach(GraphicsRectItem *item, m_programsItemsList)
    {
        if ( idList.contains(item->id()) )
        {
            item->setInCurrentHour(true);
        }
        else
        {
            item->setInCurrentHour(false);
        }
        item->update();
    }
}


void XmlDefaultHandler::evening()
{
    if ( m_main->showMode() == MainWindowImpl::Grid )
    {
        double x = QTime(0,0).secsTo( QTime(21, 30) )*(m_progWidth/1800.0);
        x = 100+x-((m_hourBeginning*2)*m_progWidth);
        m_programsView->centerOn(x ,0);
    }
    else if ( m_main->showMode() == MainWindowImpl::Channel )
    {
        foreach(GraphicsRectItem *item, m_programsItemsList)
        {
            if ( item->start().time() <= QTime(21, 30) && QTime(21, 30) < item->stop().time() )
            {
                item->ensureVisible();
                break;
            }
        }
    }
}
void XmlDefaultHandler::setDate(QDate value)
{
    if ( m_date != value )
    {
        m_date = value;
        //eveningPrograms(true);
    }
    else
        m_date = value;
}
QStringList XmlDefaultHandler::readProgrammesFromDB()
{
    clearView();
    connectDB();
    //m_query.exec("BEGIN TRANSACTION;");
    QString queryString;
    bool rc;
    QVariant v;
    //
    queryString = "select * from channels where enabled=1";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QStringList();
    }
    if ( !m_query.next() )
        return QStringList();
    // Case vide en up/gauche
    GraphicsRectItem *item = new GraphicsRectItem(m_main,
                             0,
                             QRectF(0, 0, 100, m_hourHeight),
                             QDateTime(),
                             QDateTime(),
                             "",
                             GraphicsRectItem::Channel,
                             PairIcon(),
                             0,
                             QString(),
                             false
                                                 );
    item->setZValue(10);
    v.setValue( TvProgram() );
    item->setData(0, v );
    m_programsView->scene()->addItem( item );
    m_listeItemChaines.insert(0, item );
    QStringList ids;
    do
    {
        TvChannel channel;
        channel.id = m_query.value(0).toString().replace("$", "'");
        channel.name = m_query.value(1).toString().replace("$", "'");
        channel.icon = m_query.value(2).toString().replace("$", "'");
        channel.enabled = m_query.value(3).toString().replace("$", "'").toInt();
        m_TvChannelsList << channel;
    }
    while ( m_query.next() );
    m_TvChannelsList = sortedChannels();

    QStringList titles;
    // Insertion des channels triees dans la scene
    int line = 0;
    foreach(TvChannel channel, m_TvChannelsList)
    {
        if ( channel.enabled )
        {
            ids << channel.id;
            GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                     0,
                                     QRectF(0, m_hourHeight+(line*m_progHeight), 100, m_progHeight),
                                     QDateTime(),
                                     QDateTime(),
                                     channel.name,
                                     GraphicsRectItem::Channel,
                                     PairIcon(channel.id),
                                     0,
                                     channel.id,
                                     false
                                     //PairIcon(	":/channel/"+channel.icon.section("/",-1,-1).section(".",0,0)+".png",
                                     //QPixmap(":/channel/"+channel.icon.section("/",-1,-1).section(".",0,0)+".png" )
                                     //)
                                                         );
            QObject::connect(
                m_googleImage,
                SIGNAL(imageAvailable(PairIcon)),
                item,
                SLOT(slotImageAvailable(PairIcon))
            );
            item->setZValue(17);
            m_programsView->scene()->addItem( item );
            m_listeItemChaines.append( item );
            titles << channel.id;
            line++;
        }
    }
    // Dimensionnement de la scene en fonction du nombre de demi-heure (largeur) et de channels (upeur).
    m_programsView->setSceneRect(
        m_programsView->rect().x(),
        m_programsView->rect().y(),
        //100+((48-(m_hourBeginning*2))*m_progWidth),
        100+(48*m_progWidth),
        m_hourHeight+(line*m_progHeight)
    );
    //
    m_programsView->setBackgroundBrush(QColor(Qt::red).light(188));
    // Ligne de l'heure courante
    m_currentTimeLine = new QGraphicsLineItem();
    m_currentTimeLine->setPen(QPen(QColor(Qt::red), 2, Qt::DashDotLine));
    if ( QDate::currentDate() != m_date )
        m_currentTimeLine->hide();
    m_programsView->scene()->addItem( m_currentTimeLine );
    v.setValue( TvProgram() );
    m_currentTimeLine->setData(0, v );
    m_currentTimeLine->setZValue(16);
    // Creation de la colonne des channels
    // Cadre jaune des heures
    GraphicsRectItem *hoursRect = new GraphicsRectItem(m_main,
                                  0,
                                  //QRectF(0, 0, (49-(m_hourBeginning*2))*m_progWidth, m_hourHeight),
                                  QRectF(0, 0, 49*m_progWidth, m_hourHeight),
                                  QDateTime(),
                                  QDateTime(),
                                  "",
                                  GraphicsRectItem::HourRect,
                                  PairIcon(),
                                  0,
                                  QString(),
                                  false
                                                      );
    m_programsView->scene()->addItem( hoursRect );
    hoursRect->setZValue(20);
    v.setValue( TvProgram() );
    hoursRect->setData(0, v );
    m_listeItemHeures.append( hoursRect );
    //
    QTime time(m_hourBeginning, 0);
    //for (int i=0; i<48-(m_hourBeginning*2); i++)
    for (int i=0; i<48; i++)
    {
        // Ligne pointillee pour chaque demi-heure
        QGraphicsLineItem *line = new QGraphicsLineItem(100+(i*m_progWidth), m_hourHeight, 100+(i*m_progWidth) ,m_hourHeight+(m_TvChannelsList.count()*m_progHeight));
        line->setPen(QPen(QColor(Qt::blue).light(), 1, Qt::DashDotLine));
        QVariant v;
        v.setValue( TvProgram() );
        line->setData(0, v );
        m_programsView->scene()->addItem( line );
        // Libelle de chacune des demi-heure
        GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                 0,
                                 QRectF(100+((i-1)*m_progWidth),0, m_progWidth*2, m_hourHeight),
                                 QDateTime(),
                                 QDateTime(),
                                 time.toString("hh:mm"),
                                 GraphicsRectItem::Hour,
                                 PairIcon(),
                                 0,
                                 QString(),
                                 false

                                                     );
        m_programsView->scene()->addItem( item );
        item->setZValue(21);
        v.setValue( TvProgram() );
        item->setData(0, v );
        m_listeItemHeures.append( item );
        time = time.addSecs(1800);
    }
    // Programmes
    // Reading dans la base de donnees des programs du jour choisi dans l'interface (variable m_date)
    // ainsi que du jour courant pour renseigner la fenetre "Maintenant"
    QDateTime FGFrom(m_date,QTime(m_hourBeginning,0));
    queryString = QString("select id, start, stop, channel, channelName, title, subTitle, category, director, star ")
                  + "from programs "
                  //+ "where ( date(start, 'unixepoch') = '" + m_date.toString("yyyy-MM-dd") + "'"
                  //+ "or date(stop, 'unixepoch') = '" + m_date.toString("yyyy-MM-dd") + "') ";
                  + "where ((start >= " + QString::number(FGFrom.toTime_t()) + ") and (start < " + QString::number(FGFrom.addSecs(24*60*60).toTime_t()) + "))";
                  + " or ((stop >=" + QString::number(FGFrom.toTime_t()) + ") and (stop < " + QString::number(FGFrom.addSecs(24*60*60).toTime_t()) + "))";
    QString channels = " ( ";
    foreach(TvChannel channel, m_TvChannelsList)
    {
        channels += "'" + channel.id + "' ,";
    }
    channels = channels.section(",", 0, -2) + ")";
    queryString += " and channel in " + channels;
    //QD<<queryString;
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QStringList();
    }
    if ( !m_query.next() )
        return QStringList();
    do
    {
        QDateTime start = QDateTime::fromTime_t( m_query.value(1).toInt() );
        QDateTime stop = QDateTime::fromTime_t( m_query.value(2).toInt() );
        QString channelName = m_query.value(4).toString().replace("$", "'");
        bool enabled = false;

        if ( ( start.date() == m_date || stop.date() == m_date ) )
        {
            int id = m_query.value(0).toInt();
            QString channel = m_query.value(3).toString().replace("$", "'");
            QString title = m_query.value(5).toString().replace("$", "'");
            QString subTitle = m_query.value(6).toString().replace("$", "'");
            QString director = m_query.value(8).toString().replace("$", "'");
            QString star = m_query.value(9).toString().replace("$", "'");
            QStringList categories = m_query.value(7).toString().replace("$", "'").split("|");

            int line = ids.indexOf(channel);
            double x = QDateTime(m_date).secsTo( start )*(m_progWidth/1800.0);
            x = x - ((m_hourBeginning*2)*m_progWidth);
            double w =  start.secsTo( stop )*(m_progWidth/1800.0);
            QString googleTitle;
            googleTitle += " \"" + title + "\" " ;
            if ( !subTitle.isEmpty() )
                googleTitle += " \"" + subTitle + "\" ";
            if ( !director.isEmpty() )
                googleTitle += " \"" + director + "\" ";
            GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                     id,
                                     QRectF(100+x,m_hourHeight+(line*m_progHeight),w,m_progHeight),
                                     start,
                                     stop,
                                     title,
                                     GraphicsRectItem::Program,
                                     PairIcon(googleTitle, QPixmap()), /*pairIcon(title ),*/
                                     star.section("/", 0, 0).toInt(),
                                     channel,
                                     false
                                                         );
            titles << googleTitle;
            QObject::connect(
                m_googleImage,
                SIGNAL(imageAvailable(PairIcon)),
                item,
                SLOT(slotImageAvailable(PairIcon))
            );
            item->setZValue(15);
            m_programsView->scene()->addItem( item );
            m_programsItemsList.append( item );
            m_TvProgramsList.append( id );
        }
    }
    while ( m_query.next() );
    //m_query.exec("END TRANSACTION;");
    //
    queryString = "select * from images where ok='0' order by dayOrder";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QStringList();
    }
    QList<Pair> m_imagesList;
    while ( m_query.next() )
    {
        //QD<<m_query.value(0).toString().replace("$", "'").replace("µ", "\"") << m_query.value(4).toInt();
        m_imagesList << Pair(
            m_query.value(0).toString().replace("$", "'").replace("µ", "\""),
            m_query.value(1).toString().replace("$", "'").replace("µ", "\"")
        );
    }
    if ( !m_running )
    {
        m_googleImage->setList(m_imagesList);
    }
    //listProgrammesSortedByTime();
    nowCenter();
    return titles;
}
void XmlDefaultHandler::readThumbsFromDB(QStringList list)
{
    m_googleImage->readThumbsFromDB(list);
}

bool XmlDefaultHandler::connectDB()
{
    QString dbName = m_main->iniPath() + m_main->databaseName();
    QD<<dbName;
    QSqlDatabase database;
    if ( QSqlDatabase::database(dbName).databaseName() != dbName )
    {
        database = QSqlDatabase::addDatabase("QSQLITE", dbName);
        database.setDatabaseName(dbName);
    }
    else
    {
        database = QSqlDatabase::database(dbName);
        if ( database.isOpen() )
        {
            QD << "Connect database "<<dbName;
            return true;
        }
    }
    //
    if (!database.open())
    {
        QMessageBox::critical(0, "QMagneto",
                              QObject::tr("Unable to establish a database connection.")+"\n"+
                              QObject::tr("QMagneto needs SQLite support. Please read "
                                          "the Qt SQL driver documentation for information how "
                                          "to build it."), QMessageBox::Cancel,
                              QMessageBox::NoButton);
        return false;
    }
    m_query = QSqlQuery(database);
    QD << "Connect database "<<dbName;
    return true;
}


void XmlDefaultHandler::imageToTmp(QString title, bool isChannel)
{
    connectDB();
    QString queryString = "select * from images where title = '" + title.replace("'", "$").replace("\"", "µ")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return;
    }
    if ( m_query.next() )
    {
        QFile file;
        QByteArray data = m_query.value(3).toByteArray();
        QImage image;
        if ( !data.isEmpty() )
            image = QImage::fromData( data  ) ;
        if ( image.isNull() )
            return;
        if ( isChannel )
            file.setFileName(QDir::tempPath()+"/qmagnetochannel.jpg");
        else {
	        image = image.scaledToHeight(100, Qt::SmoothTransformation);
            file.setFileName(QDir::tempPath()+"/qmagnetoprog.jpg");
       	}
        if (!file.open(QIODevice::WriteOnly ))
        {
            QD << "pb" << file.fileName();
            return;
        }
        //QD<<icon<<image.isNull()<<image.size();
        image.save(&file, "JPG");
        file.close();
        while ( m_query.next() )
        {}
    }
}



PairIcon XmlDefaultHandler::pairIcon(QString title)
{
    PairIcon pair;
    QString queryString = "select * from images where ok=1 and title = '" + QString(title).replace("'", "$").replace("\"", "µ")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return pair;
    }

    if ( m_query.next() )
    {
        QByteArray data = m_query.value(3).toByteArray();
        QImage image;
        if ( !data.isEmpty() )
            image = QImage::fromData( data );//uncompress
        pair = PairIcon(title,
                        QPixmap::fromImage( image )
                       );
    }
    while ( m_query.next() )
    {}
    return pair;
}


void XmlDefaultHandler::nowCenter()
{
    if ( m_main->showMode() == MainWindowImpl::Grid )
    {
        double x = QTime(0,0).secsTo( QTime::currentTime() )*(m_progWidth/1800.0);
        x = 100+x-((m_hourBeginning*2)*m_progWidth);
        m_programsView->centerOn(x ,0);
    }
    else if ( m_main->showMode() == MainWindowImpl::Channel )
    {
        QDateTime now = QDateTime::currentDateTime();
        foreach(GraphicsRectItem *item, m_programsItemsList)
        {
            if ( item->start() <= now && now < item->stop() )
            {
                item->ensureVisible();
                break;
            }
        }
    }
}


QDate XmlDefaultHandler::minimumDate()
{
    QString queryString = "select min(start) from programs";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QDate();
    }
    if ( !m_query.next() )
        return QDate();
    return QDateTime::fromTime_t( m_query.value(0).toInt() ).date();
}


QDate XmlDefaultHandler::maximumDate()
{
    QString queryString = "select max(start) from programs";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QDate();
    }
    if ( !m_query.next() )
        return QDate();
    return QDateTime::fromTime_t( m_query.value(0).toInt() ).date();
}


QList<TvChannel> XmlDefaultHandler::sortedChannels()
{
    // On tri les channels par numero de id
    QList<TvChannel> sortedList;
    //QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    //settings.beginGroup("Channels");
    int i=0;
    do
    {
        //QString id = settings.value("pos"+QString::number(i++)).toString();
        if ( m_sortedChannelsList.count()-1 < i )
            break;
        QString id = m_sortedChannelsList[i++];
        int n = 0;
        int index = -1;
        foreach( TvChannel chaineTV, m_TvChannelsList)
        {
            if ( chaineTV.id == id )
            {
                index = n;
                break;
            }
            n++;
        }
        if ( index != -1 )
            sortedList.append(m_TvChannelsList.at(index));
        m_TvChannelsList.removeAt(index);
    }
    while ( m_TvChannelsList.count() );
    // Maintenant les channels non presentes dans le fichier ini
    foreach( TvChannel chaineTV, m_TvChannelsList)
    {
        sortedList.append(chaineTV);
    }
    m_TvChannelsList = sortedList;
    //settings.endGroup();
    return sortedList;
}


QList<TvProgram> XmlDefaultHandler::sortedPrograms(QList<TvProgram> list)
{
    // On tri les programs par numero de id de la channel
    if ( !list.count() )
        return QList<TvProgram>();
    QList<TvProgram> sortedList;
    //QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    //settings.beginGroup("Channels");
    int i=0;
    //do
    foreach(QString channel, m_sortedChannelsList)
    {
        //QString channel = settings.value("pos"+QString::number(i++)).toString();
        //QString channel = m_sortedChannelsList[i++];
        int n = 0;
        int index = -1;
        foreach( TvProgram programTV, list)
        {
            if ( programTV.channel == channel )
            {
                index = n;
                break;
            }
            n++;
        }
        if ( index != -1 )
            sortedList.append(list.at(index));
        list.removeAt(index);
    }
    //while ( list.count() );
    // Maintenant les channels non presentes dans le fichier ini
    //foreach( TvProgram programTV, list)
    //{
    //sortedList.append(programTV);
    //}
    //settings.endGroup();
    return sortedList;
}


void XmlDefaultHandler::clearView()
{
    m_listeItemChaines.clear();
    m_listeItemHeures.clear();
    m_programsItemsList.clear();
    // Suppression de tous les elements dans la vue
    QList<QGraphicsItem *> list = m_programsView->scene()->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        if ( *it )
        {
            m_programsView->scene()->removeItem(*it);
            delete *it;
        }
    }
}


GraphicsRectItem * XmlDefaultHandler::findProgramme(QString text, bool backward, bool fromBegin, bool sensitive, bool wholeWord)
{
    if ( wholeWord )
        text = "\\b("+text+")\\b";
    QRegExp regExp( text );
    regExp.setCaseSensitivity( ( Qt::CaseSensitivity) sensitive );
    QListIterator<GraphicsRectItem *> iterator(m_programsItemsList);
    GraphicsRectItem *item = 0;
    bool found = false;
    if ( !fromBegin )
    {
        if ( backward )
        {
            iterator.toBack();
            while (iterator.hasPrevious())
            {
                item = iterator.previous();
                if ( item->isEnabled() )
                {
                    //TvProgram prog = item->data(0).value<TvProgram>();
                    found = true;
                    break;
                }
            }
        }
        else
        {
            iterator.toFront();
            while (iterator.hasNext())
            {
                item = iterator.next();
                if ( item->isEnabled() )
                {
                    //TvProgram prog = item->data(0).value<TvProgram>();
                    found = true;
                    break;
                }
            }
        }
    }
    if ( backward )
    {
        if ( !found )
        {
            iterator.toBack();
        }
        if ( item )
        {
            //TvProgram prog = item->data(0).value<TvProgram>();
        }
        while (iterator.hasPrevious())
        {
            item = iterator.previous();
            //TvProgram prog = item->data(0).value<TvProgram>();
            if ( item->text().contains(regExp) && item->stop().time() >= QTime(m_hourBeginning, 0) )
            {
                double x = QTime(0,0).secsTo( item->start().time() )*(m_progWidth/1800.0);
                x = 100+x-((m_hourBeginning*2)*m_progWidth);
                QStringList ids;
                foreach(TvChannel channel, m_TvChannelsList)
                {
                    if ( channel.enabled )
                    {
                        ids << channel.id;
                    }
                }
                int line = ids.indexOf(item->channel());
                double y = m_hourHeight+(line*m_progHeight);
                m_programsView->centerOn(x ,y);
                return item;
            }
        }
    }
    else
    {
        if ( !found )
        {
            iterator.toFront();
        }
        while (iterator.hasNext())
        {
            item = iterator.next();
            if ( item->text().contains(regExp) && (item->stop().time() >= QTime(m_hourBeginning, 0) || item->stop().date()>m_date ) )
            {
                double x = QTime(0,0).secsTo( item->start().time() )*(m_progWidth/1800.0);
                x = 100+x-((m_hourBeginning*2)*m_progWidth);
                QStringList ids;
                foreach(TvChannel channel, m_TvChannelsList)
                {
                    if ( channel.enabled )
                    {
                        ids << channel.id;
                    }
                }
                int line = ids.indexOf(item->channel());
                double y = m_hourHeight+(line*m_progHeight);
                m_programsView->centerOn(x ,y);
                return item;
            }
        }
    }
    return 0;
}

void XmlDefaultHandler::listProgrammesSortedByTime()
{
    m_programsSortedItemsList.clear();
    QString queryString = QString("select * ")
                          + "from programs "
                          + "where date(start, 'unixepoch') = '" + m_date.toString("yyyy-MM-dd") + "'"
                          + "or date(stop, 'unixepoch') = '" + m_date.toString("yyyy-MM-dd") + "'";

    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
    }
    if ( !m_query.next() )
        return;
    do
    {
        TvProgram prog;
        prog.start = QDateTime::fromTime_t( m_query.value(0).toInt() );
        prog.stop = QDateTime::fromTime_t( m_query.value(1).toInt() );
        prog.channel = m_query.value(2).toString().replace("$", "'");
        foreach(GraphicsRectItem *item, m_programsItemsList)
        {
            TvProgram p = item->data(0).value<TvProgram>();
            if ( prog.start == p.start && prog.stop == p.stop && prog.channel == p.channel )
            {
                m_programsSortedItemsList.append(item);
            }
        }
    }
    while ( m_query.next() );
}

bool XmlDefaultHandler::programOutdated(int day)
{
    QString queryString = "select count(*) from programs where "
                          " start >= '" + QString::number(QDateTime::currentDateTime().addDays(day).toTime_t())
                          + "' and start < '" + QString::number(QDateTime::currentDateTime().addDays(day+1).addSecs(-60).toTime_t()) + "'";

    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return true;
    }
    if ( !m_query.next() )
        return true;
    int count = m_query.value(0).toInt();
    return count == 0;
}


TvProgram XmlDefaultHandler::tvProgram(int id)
{
    QString queryString = "select * from programs where id="+QString::number(id);
    //QD<< queryString;
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return TvProgram();
    }
    if ( !m_query.next() )
        return TvProgram();
    TvProgram prog;
    prog.programId = m_query.value(0).toInt();
    prog.start = QDateTime::fromTime_t( m_query.value(1).toInt() );
    prog.before = 10;

    prog.stop = QDateTime::fromTime_t( m_query.value(2).toInt() );
    prog.after = 10;
    prog.channel = m_query.value(3).toString().replace("$", "'");
    prog.channelName = m_query.value(4).toString().replace("$", "'");
    prog.title = m_query.value(5).toString().replace("$", "'");
    prog.subTitle = m_query.value(6).toString().replace("$", "'");
    prog.category = m_query.value(7).toString().replace("$", "'").split("|");
    prog.resume = QString::fromUtf8(m_query.value(8).toByteArray()).replace("$", "'").split("|");
    prog.story = QString::fromUtf8(m_query.value(9).toByteArray()).replace("$", "'");
    //prog.resume = QString::fromUtf8(qUncompress(m_query.value(8).toByteArray())).replace("$", "'").split("|");
    //prog.story = QString::fromUtf8(qUncompress(m_query.value(9).toByteArray())).replace("$", "'");
    prog.aspect = m_query.value(10).toString().replace("$", "'");
    prog.credits = m_query.value(11).toString().replace("$", "'");
    prog.director = m_query.value(12).toString().replace("$", "'");
    prog.actors = m_query.value(13).toString().replace("$", "'").split("|");
    prog.date = m_query.value(14).toString().replace("$", "'");
    prog.star = m_query.value(15).toString().replace("$", "'");
    prog.icon = m_query.value(16).toString().replace("$", "'");
    return prog;
}


QStringList XmlDefaultHandler::getSortedChannelsList()
{
    return m_sortedChannelsList;
}


void XmlDefaultHandler::setSortedChannelsList()
{
    m_sortedChannelsList.clear();
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    int i=0;
    QString channel;
    do
    {
        channel = settings.value("pos"+QString::number(i++)).toString();
        m_sortedChannelsList << channel;
    }
    while ( !channel.isEmpty());
    settings.endGroup();
}


bool XmlDefaultHandler::connectNewDB()
{
    QString name = m_main->databaseName();
    if ( name == "qmagnetoa.db" )
    {
        name = "qmagnetob.db";
    }
    else
    {
        name = "qmagnetoa.db";
    }
    QString dbName = m_main->iniPath() + name;
    QSqlDatabase database;
    if ( QSqlDatabase::database(dbName).databaseName() != dbName )
    {
        database = QSqlDatabase::addDatabase("QSQLITE", dbName);
        database.setDatabaseName(dbName);
    }
    else
    {
        database = QSqlDatabase::database(dbName);
        if ( database.isOpen() )
        {
            QD << "Connect database "<<dbName;
            return true;
        }
    }
    //
    if (!database.open())
    {
        QMessageBox::critical(0, "QMagneto",
                              QObject::tr("Unable to establish a database connection.")+"\n"+
                              QObject::tr("QMagneto needs SQLite support. Please read "
                                          "the Qt SQL driver documentation for information how "
                                          "to build it."), QMessageBox::Cancel,
                              QMessageBox::NoButton);
        return false;
    }
    m_queryNewBase = QSqlQuery(database);
    QD << "Connect database "<<dbName;
    return true;
}


void XmlDefaultHandler::setEnableChannel(QString name, bool enable)
{
    m_query.prepare("update channels set enabled='"
                    +QString::number(enable)
                    +"'where id='" +name+ "'");
    bool rc = m_query.exec();
    if (rc == false)
    {
        QD << "Failed to insert record to db" << m_query.lastError();
    }
}


QList<TvChannel> XmlDefaultHandler::disabledChannels()
{
    QList<TvChannel> list;
    QString queryString = "select * from channels where enabled=0";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return list;
    }
    if ( !m_query.next() )
        return list;
    do
    {
        TvChannel channel;
        channel.id = m_query.value(0).toString().replace("$", "'");
        channel.name = m_query.value(1).toString().replace("$", "'");
        channel.icon = m_query.value(2).toString().replace("$", "'");
        channel.enabled = m_query.value(3).toString().replace("$", "'").toInt();
        list << channel;
    }
    while ( m_query.next() );
    return list;
}

void XmlDefaultHandler::writeThumbnailInDB(QVariant clob, QString title, bool create)
{
    connectDB();
    if ( create )
    {
        m_query.exec("delete from images where title='"+title+"'");
        m_query.prepare("INSERT INTO images (title, url, ok, data, dayOrder)"
                        "VALUES (:title, :url, :ok, :data, :dayOrder)");
        m_query.bindValue(":title", QString(title).replace("'", "$").replace("\"", "µ"));
        m_query.bindValue(":url", QString());
        m_query.bindValue(":ok","1");
        m_query.bindValue(":data", clob);
        int dayOrder = abs(QDateTime::currentDateTime().secsTo(m_programTV.start));
        m_query.bindValue(":dayOrder", dayOrder);
        bool rc = m_query.exec();
        if (rc == false)
        {
            QD << "Failed to insert record to db" << m_query.lastError();
            //QD << queryString;
        }

    }
    m_query.prepare("update images set ok='1', data=:data where title='" +title.replace("'", "$").replace("\"", "µ")+ "'");
    m_query.bindValue(":data", clob);
    bool rc = m_query.exec();
    if (rc == false)
    {
        QD << "Failed to insert record to db" << m_query.lastError();
    }
    QD << "download ok for:" << title;

}


QSqlQuery XmlDefaultHandler::query(QString s)
{
    bool rc = m_query.exec(s);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << s;
        return QSqlQuery();
    }
    return m_query;
}


QStringList XmlDefaultHandler::readChannelFromDB(QString channelName)
{
    m_programsView->show();
    m_programsItemsList.clear();
    clearView();
    connectDB();
    QString queryString;
    bool rc;
    QVariant v;
    //
    queryString = "select * from channels where enabled=1";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QStringList();
    }
    if ( !m_query.next() )
        return QStringList();
    TvChannel theChannel;
    QStringList ids;
    do
    {
        TvChannel channel;
        channel.id = m_query.value(0).toString().replace("$", "'");
        channel.name = m_query.value(1).toString().replace("$", "'");
        channel.icon = m_query.value(2).toString().replace("$", "'");
        channel.enabled = m_query.value(3).toString().replace("$", "'").toInt();
        m_TvChannelsList << channel;
        if ( channel.id == channelName )
        {
            theChannel.id = m_query.value(0).toString().replace("$", "'");
            theChannel.name = m_query.value(1).toString().replace("$", "'");
            theChannel.icon = m_query.value(2).toString().replace("$", "'");
            theChannel.enabled = m_query.value(3).toString().replace("$", "'").toInt();
        }
    }
    while ( m_query.next() );
    m_TvChannelsList = sortedChannels();
    m_programsView->setBackgroundBrush(QColor(Qt::red).light(188));
    // Ligne de l'heure courante
    m_currentTimeLine = new QGraphicsLineItem();
    m_currentTimeLine->setPen(QPen(QColor(Qt::red), 2, Qt::DashDotLine));
    m_currentTimeLine->hide();
    m_programsView->scene()->addItem( m_currentTimeLine );
    v.setValue( TvProgram() );
    m_currentTimeLine->setData(0, v );
    m_currentTimeLine->setZValue(16);
    // Creation de la colonne des channels
    // Programmes
    // Reading dans la base de donnees des programs du jour choisi dans l'interface (variable m_date)
    // ainsi que du jour courant pour renseigner la fenetre "Maintenant"
    queryString = QString("select id, start, stop, channel, channelName, title, subTitle, category, director, star ")
                  + "from programs "
                  + "where (date(start, 'unixepoch') = '" + m_date.toString("yyyy-MM-dd") + "'"
                  + "or date(stop, 'unixepoch') = '" + m_date.toString("yyyy-MM-dd") + "')";
    queryString += " and channel in ('" + channelName + "')";
    ;
    QD<<queryString;
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        //QD << queryString;
        return QStringList();
    }
    if ( !m_query.next() )
        return QStringList();
    int line = 0;
    QStringList titles;
    do
    {
        QDateTime start = QDateTime::fromTime_t( m_query.value(1).toInt() );
        QDateTime stop = QDateTime::fromTime_t( m_query.value(2).toInt() );
        QString channelName = m_query.value(4).toString().replace("$", "'");
        bool enabled = false;

        if ( ( start.date() == m_date || stop.date() == m_date ) /*&& enabled*/ )
        {
            int id = m_query.value(0).toInt();
            QString channel = m_query.value(3).toString().replace("$", "'");
            QString title = m_query.value(5).toString().replace("$", "'");
            QString subTitle = m_query.value(6).toString().replace("$", "'");
            QString director = m_query.value(8).toString().replace("$", "'");
            QString star = m_query.value(9).toString().replace("$", "'");
            QStringList categories = m_query.value(7).toString().replace("$", "'").split("|");

            double x = 20.0;
            double w = m_programsView->viewport()->rect().width()-20;//  650;
            QString googleTitle;
            googleTitle += " \"" + title + "\" " ;
            if ( !subTitle.isEmpty() )
                googleTitle += " \"" + subTitle + "\" ";
            if ( !director.isEmpty() )
                googleTitle += " \"" + director + "\" ";
            GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                     id,
                                     QRectF(x,(line*m_progHeight),w,m_progHeight),
                                     start,
                                     stop,
                                     title,
                                     GraphicsRectItem::Program,
                                     PairIcon(googleTitle, QPixmap()), /*pairIcon(title ),*/
                                     star.section("/", 0, 0).toInt(),
                                     channel,
                                     false
                                                         );
            titles << googleTitle;
            QObject::connect(
                m_googleImage,
                SIGNAL(imageAvailable(PairIcon)),
                item,
                SLOT(slotImageAvailable(PairIcon))
            );
            m_programsView->scene()->addItem( item );
            m_programsItemsList.append( item );
            m_TvProgramsList.append( id );
            line++;
        }
    }
    while ( m_query.next() );
    //
    queryString = "select * from images where ok='0' order by dayOrder";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QStringList();
    }
    QList<Pair> m_imagesList;
    while ( m_query.next() )
    {
        m_imagesList << Pair(
            m_query.value(0).toString().replace("$", "'").replace("µ", "\""),
            m_query.value(1).toString().replace("$", "'").replace("µ", "\"")
        );
    }
    if ( !m_running )
    {
        m_googleImage->setList(m_imagesList);
    }
    m_programsView->setSceneRect(m_programsView->scene()->itemsBoundingRect().adjusted(0,0,0,250) );
    return titles;
}


void XmlDefaultHandler::setPositionOnChannelMode(QGraphicsView *view)
{
    int y = 0;
    QList<GraphicsRectItem *> list;
    if ( view == m_programsView )
        list = m_programsItemsList;
    else
        list = m_main->findGlobalImpl()->listItemProgrammes();
    foreach(GraphicsRectItem *item, list)
    {
        int w = view->viewport()->width()-30;
        item->setRect(view->sceneRect().x()+15, view->sceneRect().y()+y, w , item->rect().height());
        y += item->rect().height();
    }
}


void XmlDefaultHandler::expandItem(GraphicsRectItem *item, bool expand)
{
    if ( expand )
    {}
    else
    {
        item->setRect(item->rect().x(), item->rect().y(), item->rect().width(), m_progHeight);
    }
    item->update();
    setPositionOnChannelMode(item->scene()->views().first());
}


QStringList XmlDefaultHandler::categories(bool forceReading)
{
    if ( !forceReading && m_categories.count() )
        return m_categories;
    QTime t;
    t.start();
    QString channels = " ( ";
    foreach(TvChannel channel, m_TvChannelsList)
    {
        channels += "'" + channel.id + "' ,";
    }
    channels = channels.section(",", 0, -2) + ")";
    QString queryString = "select category from programs where channel in " + channels + " group by category";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return QStringList();
    }
    if ( !m_query.next() )
        return QStringList();
    m_categories.clear();
    do
    {
        foreach(QString category,  m_query.value(0).toString().replace("$", "'").split("|"))
        {
            if ( !m_categories.contains(category) )
                m_categories << category;
        }
    }
    while ( m_query.next() );
    QD<<t.elapsed();
    return m_categories;
}



int XmlDefaultHandler::programId(QString channel, int start)
{
    connectDB();
    QString queryString = "select id from programs where start="+QString::number(start)+ " and channel='"+channel+"'";
    QD<<queryString;
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return -1;
    }
    if ( !m_query.next() )
        return -1;
    return m_query.value(0).toInt();
}
void XmlDefaultHandler::showExpandedPixmap(QRect pos, QPixmap pixmap)
{
    if ( m_main->showMode() == MainWindowImpl::Channel || !m_main->expandPixmap() ) {
    	return;
   	}
	QPixmap newPix(pixmap.size().width()+4, pixmap.size().height()+4);
	QPainter painter(&newPix);
	newPix.fill(Qt::black);
	painter.drawPixmap(2, 2, pixmap);
	painter.end();
	ExpandedPixmap *item = new ExpandedPixmap(newPix, this);
	item->setPos(pos.x(), pos.y());
	item->setZValue(500);
	item->setAcceptHoverEvents( true );
	m_programsView->scene()->addItem( item );
	item->ensureVisible();
}

void XmlDefaultHandler::hideExpandedPixmap(ExpandedPixmap *item)
{
	delete item;
}

