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
#include "googleimage.h"
#include "mainwindowimpl.h"
#include "xmldefaulthandler.h"
#include <QDir>
#include <QUrl>
#include <QSqlError>
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

GoogleImage::GoogleImage(MainWindowImpl *parent, XmlDefaultHandler *handler)
        : QObject(parent), m_main(parent), m_handler(handler)
{
    m_stop = false;
    //httpURL = 0;
    //m_httpThumbnail = 0;
    //html=new QString();
    //resultHtml=new QString();
    rx_start.setPattern("setResults[(][[]");

    rx_data.setPattern("[[]\".*[]],");
    rx_data.setMinimal(true);

    rx_href.setPattern("(http://.*)[\\\\|\"]");
    rx_href.setMinimal(true);

    rx_other.setPattern(",[\"](.*)[\"]");
    rx_other.setMinimal(true);


}
void GoogleImage::setList(QList<Pair> list)
{
    if ( !m_main->groupGoogleImage() )
    {
        m_list = QList<Pair>();
        return;
    }
    m_time.start();
    m_stop = false;
    m_list = list;
    //for(int i=0; i<10; i++)
    {
        if ( m_list.count() )
            google_search(m_list.first());
    }
}
//
GoogleImage::~GoogleImage()
{}
void GoogleImage::google_search(Pair pp)
{
    QString s,search_string,content_type,image_size,coloration,site_search,safeFilter,search_url;
    //QD<<pp.first<<pp.second;
    if ( !pp.second.isEmpty() )
    {
        m_pair = Pair(m_list.first().first, pp.second);
        getThumbnail(pp.second);
        return;
    }
    search_string=pp.first.simplified().replace(" ","+");
    search_url="http://images.google.com/images?q="+search_string+"&safe=active";
    QNetworkRequest request(search_url);
    reply = manager.get(request);
    connect(reply, SIGNAL(finished()),
            SLOT(httpURL_done()));

    QD << "Searching URL:"<< search_url;
}
//
void GoogleImage::httpURL_done()
{
    qApp->processEvents();
    if (reply->error())
    {
        QD << reply->error();
        m_list.pop_front();
        if ( m_list.count() )
            google_search(m_list.first());
    }
    else
    {
        QByteArray r;
        r=QByteArray::fromPercentEncoding( reply->readAll() );
        QString URL = parse_html(QString::fromUtf8(r));

        QD << "URL " << URL;
        if ( m_list.count() )
        {
            if ( !URL.isEmpty() )
            {
                m_pair = Pair(m_list.first().first, URL);
                getThumbnail(URL);
            }
            else
            {
                m_list.pop_front();
                if ( m_list.count() )
                    google_search(m_list.first());
            }
        }
    }
}
//
QString GoogleImage::parse_html(QString html)
{
    QRegExp rx;
    rx.setPattern("(q=tbn.*)\"");
    rx.setMinimal(true);
	int poss = 0;
    if ((poss = rx.indexIn( html, poss)) != -1)
    {
    	return "http://t0.gstatic.com/images?" + rx.cap(1);
    }
    return QString();
}
//
void GoogleImage::getThumbnail(QString URL)
{
    qApp->processEvents();
    QD << "getThumbnail" <<URL << m_list.first().second;
    QNetworkRequest request(URL);
    reply = manager.get(request);
    connect(reply, SIGNAL(finished()),
            SLOT(httpThumbnail_done()));
}
//
void GoogleImage::httpThumbnail_done()
{
    qApp->processEvents();
    if ( reply->error() )
    {
        QD << reply->error();
    }
    else
    {
        QByteArray data;
        data=reply->readAll();

        if ( !data.isEmpty() )
        {
            QVariant clob(data);
            m_handler->writeThumbnailInDB(clob, m_pair.first);
            QD << m_pair.first << m_pair.second;
            emit imageAvailable(
                PairIcon(
                    m_pair.first,
                    QPixmap::fromImage( QImage::fromData( ( data ) ) )
                )
            );
        }
    }
    if ( m_list.count() )
        m_list.pop_front();
    if ( m_list.count() )
        google_search(m_list.first());
    else
        QD << "Liste vide "<<m_time.elapsed();
}
//
void GoogleImage::stop()
{
    m_stop = true;
    m_list.clear();
    QD << "stop" << m_list.count();
}

PairIcon GoogleImage::pairIcon(QString icon)
{
    PairIcon pair = m_handler->pairIcon(icon);
    if ( !pair.pixmap().isNull() )
        emit imageAvailable( pair );
    return pair;
}

void GoogleImage::readThumbsFromDB(QStringList list)
{
    m_stop = false;
    foreach(QString s, list)
    {
        if ( m_stop )
            break;
        pairIcon(s);
        qApp->processEvents();
    }
}

