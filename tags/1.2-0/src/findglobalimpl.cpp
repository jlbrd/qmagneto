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

#include "findglobalimpl.h"
#include "xmldefaulthandler.h"
#include "graphicsrectitem.h"
#include "mainwindowimpl.h"
#include <QHeaderView>
#include <QListWidgetItem>
//
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

FindGlobalImpl::FindGlobalImpl( QWidget * parent, XmlDefaultHandler *handler, Qt::WFlags f)
        : QDialog(parent, f), m_handler(handler)
{
    m_main = (MainWindowImpl *) parent;
    setupUi(this);
    setWindowFlags( windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint );
    connect(today, SIGNAL(stateChanged ( int  )), this, SLOT(slotHours(int)) );
    connect(groupHours, SIGNAL(clicked ( bool  )), this, SLOT(slotHours(bool)) );
    startDate->setDate(QDate::currentDate());
    stopDate->setDate(QDate::currentDate());
    stopTime->setTime(QTime::fromString("23:59", "hh:mm"));
    view->setScene( new QGraphicsScene(this) );
    view->setBackgroundBrush(QColor(Qt::green).light(188));
}
//
void FindGlobalImpl::setCategories(QStringList value)
{
    m_categories = value;
    categories->clear();
    foreach(QString categ, m_categories)
    {
        QListWidgetItem *newItem;
        newItem = new QListWidgetItem( categ );
        newItem->setCheckState(Qt::Unchecked);
        newItem->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsEnabled );
        categories->addItem(newItem);
    }
}

void FindGlobalImpl::on_findButton_clicked()
{
    m_programsItemsList.clear();
    QList<QGraphicsItem *> list = view->scene()->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        if ( *it )
        {
            view->scene()->removeItem(*it);
            delete *it;
        }
    }
    delete view->scene();
    view->setScene( new QGraphicsScene(this) );
    view->setBackgroundBrush(QColor(Qt::green).light(188));
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    //view->centerOn(QPointF(0, 0));

    QString query = "select * from programs where 1 ";
    // Categories
    if ( groupCategories->isChecked() )
    {

        bool useCategories = false;
        QString in = " and ( ";
        for (int i=0; i<categories->count(); i++)
        {
            QListWidgetItem *item = categories->item(i);
            if ( item->checkState() == Qt::Checked )
            {
                in += " category like '%" + item->text() + "%' OR ";
                useCategories = true;
            }
        }
        in = in.section("OR", 0, -2);
        in += " ) ";
        if ( useCategories )
            query += in;
    }
    if ( groupTitle->isChecked() )
    {
        query += " AND title like '%" + title->text().replace("'", "$") + "%' ";
    }
    if ( today->isChecked() )
    {
        QString tyesterday = QString::number(QDateTime(QDate::currentDate().addDays(-1), QTime(0,0)).toTime_t());
        QString tnow = QString::number(QDateTime(QDate::currentDate(), QTime::currentTime() ).toTime_t());
        QString ttoday = QString::number(QDateTime(QDate::currentDate(), QTime(0,0)).toTime_t());
        QString ttomorrow = QString::number(QDateTime(QDate::currentDate().addDays(1), QTime(0,0)).toTime_t());
        QString ttomorrowEvening = QString::number(QDateTime(QDate::currentDate().addDays(1), QTime(23,59)).toTime_t());
        // Programmes commencant entre hier et ce soir 23h59 et finissant entre aujourd'hui 0h00 et demain 0h00
        query += " and ( ( start >= '" + tnow + "' and start < '" + ttomorrow + "' "
                 + " and stop >= '" + ttoday + "' and stop <= '" + ttomorrow + "' )"
                 // ou commencant aujourd'hui et finissant apres demain 0h00
                 + " or ( start >= '" + ttoday + "' and start < '" + ttomorrow + "' "
                 + " and stop >= '" + ttomorrow + "' and stop <= '" + ttomorrowEvening + "' ) )";
    }
    else if ( groupHours->isChecked() )
    {
        query += " and start >= '" + QString::number(QDateTime(startDate->date(), startTime->time()).toTime_t())
                 + "' and start <= '" + QString::number(QDateTime(stopDate->date(), stopTime->time()).toTime_t()) + "' ";
    }
    query += " order by start ";
    QD << query;
    // Query
    QSqlQuery res = m_handler->query(query);
    if ( !res.next() )
        return;
    int line = 0;
    double m_progHeight = 70.0;
    double m_hourHeight = 25.0;
    do
    {
        int id = res.value(0).toInt();
        QDateTime start = QDateTime::fromTime_t( res.value(1).toInt() );
        QDateTime stop = QDateTime::fromTime_t( res.value(2).toInt() );
        QString channel = res.value(3).toString().replace("$", "'");
        QString channelName = res.value(4).toString().replace("$", "'");
        QString title = res.value(5).toString().replace("$", "'");
        QString subTitle = res.value(6).toString().replace("$", "'");
        QString director = res.value(12).toString().replace("$", "'");
        QString star = res.value(15).toString().replace("$", "'");

        QString googleTitle;
        //googleTitle += " \"" + channelName + "\" ";
        googleTitle += " \"" + title + "\" " ;
        if ( !subTitle.isEmpty() )
            googleTitle += " \"" + subTitle + "\" ";
        if ( !director.isEmpty() )
            googleTitle += " \"" + director + "\" ";

        GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                 id,
                                 QRectF(0,m_hourHeight+(line*m_progHeight),view->viewport()->width()-102,m_progHeight),
                                 start,
                                 stop,
                                 title,
                                 GraphicsRectItem::Program,
                                 m_handler->pairIcon(googleTitle ),
                                 star.section("/", 0, 0).toInt(),
                                 channel,
                                 true
                                                     );
        view->scene()->addItem( item );
        m_programsItemsList << item;
        line++;
    }
    while ( res.next() );
    //QGraphicsLineItem *lineItem = new QGraphicsLineItem(0,m_hourHeight+(line*m_progHeight),view->viewport()->width()-1,m_hourHeight+(line*m_progHeight));
    //lineItem->setZValue( -15 );
    //view->scene()->addItem( lineItem );
    view->update();
    view->setSceneRect(view->scene()->itemsBoundingRect().adjusted(0,0,0,250) );
    m_handler->setPositionOnChannelMode(view);
    m_main->emitShowIconsStatus();
}

void FindGlobalImpl::slotHours(bool value)
{
    slotHours( (int)value );
}
void FindGlobalImpl::slotHours(int value)
{
    if ( sender() == today )
    {
        if ( today->isChecked() )
            groupHours->setChecked( !today->isChecked() );
    }
    else
    {
        if ( groupHours->isChecked() )
            today->setChecked( !groupHours->isChecked() );

    }
}
void FindGlobalImpl::resizeEvent(QResizeEvent * event)
{
    QDialog::resizeEvent( event );
    view->update();
    view->setSceneRect(view->scene()->itemsBoundingRect() );
    m_handler->setPositionOnChannelMode(view);
}

