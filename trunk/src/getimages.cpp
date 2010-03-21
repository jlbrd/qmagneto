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
* Program URL   : http://biord-software.org/qmagneto/
*
*/

#include "getimages.h"
#include <QFile>
#include <QVariant>
#include <QSqlError>
#include <QImage>
#include <QDir>
#include <QUrl>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
GetImages::GetImages()
        : QObject()
{
    m_http = 0;
}
//
GetImages::~GetImages()
{
    if ( m_http )
    {
        m_http->clearPendingRequests();
        m_http->abort();
        m_http->close();
        m_http->deleteLater();
    }
}
void GetImages::slotRequestFinished(bool err)
{
    if ( err )
    {
        QD << m_http->errorString();
        return;
    }
    if ( !m_pairList.count() )
        return;
    QByteArray data;
    data = m_http->readAll();
    if ( data.isEmpty() )
        return;
    QVariant clob(data);
    m_query.prepare("update images set ok='1', data=:data where icon='" +m_pairList.first().first.replace("'", "$")+ "'");
    m_query.bindValue(":data", clob);
    bool rc = m_query.exec();
    if (rc == false)
    {
        qDebug() << "Failed to insert record to db" << m_query.lastError();
    }
    emit imageAvailable(
        PairIcon(
            m_pairList.first().first,
            QPixmap::fromImage( QImage::fromData( ( data ) ) )
        )
    );
    QD << tr("download ok for:") << m_pairList.first().first << tr("size:") << data.size();
    m_http->close();
    if ( m_pairList.count() )
    {
        m_pairList.pop_front();
        get();
    }
}

void GetImages::get()
{
    if ( m_pairList.count() == 0 )
        return;
    QString icon = m_pairList.first().second;
    QUrl url(icon);
    m_http->setHost(url.host());
    m_http->get( url.toString());
}


void GetImages::imageToTmp(QString icon, QSqlQuery query, bool isChannel)
{
    m_query = query;
    QString queryString = "select * from images where icon = '" + icon.replace("'", "$")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return;
    }
    if ( m_query.next() )
    {
        QFile file;
        QImage image = QImage::fromData( ( m_query.value(2).toByteArray() ) );
        if ( isChannel )
            file.setFileName(QDir::tempPath()+"/qmagnetochannel.jpg");
        else
            file.setFileName(QDir::tempPath()+"/qmagnetoprog.jpg");
        if (!file.open(QIODevice::WriteOnly ))
            return;
        //file.write(m_query.value(2).toByteArray());
         image.save(&file, "JPG"); 
        file.close();
        while( m_query.next() );
    }
}


void GetImages::setList(QList<Pair> list, QSqlQuery query, QString proxyAddress, int proxyPort, QString proxyUsername, QString proxyPassword)
{
    if ( m_http )
    {
        m_http->clearPendingRequests();
        m_http->abort();
        m_http->close();
        delete m_http;
        m_http = 0;
    }
    m_pairList = list;

    if ( m_pairList.count() )
QD<< m_pairList.count();    {
        m_http = new QHttp( this );
        if ( !proxyAddress.isEmpty() )
        {
            m_http->setProxy(proxyAddress, proxyPort, proxyUsername, proxyPassword);
        }
        m_query = query;
        connect(m_http, SIGNAL(done(bool)), this, SLOT(slotRequestFinished(bool)) );
        get();
    }
}


PairIcon GetImages::pairIcon(QString icon, QSqlQuery query)
{
    PairIcon pair;
    m_query = query;
    QString queryString = "select * from images where icon = '" + icon.replace("'", "$")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return pair;
    }

    if ( m_query.next() )
    {
QD<<queryString;
        pair = PairIcon(icon,
                        QPixmap::fromImage( QImage::fromData( ( m_query.value(2).toByteArray() ) ) )
                       );
    }
    return pair;
}
