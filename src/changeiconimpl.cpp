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

#include "mainwindowimpl.h"
#include "changeiconimpl.h"
#include "channeliconitem.h"
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
//
ChangeIconImpl::ChangeIconImpl( QWidget * parent, QString filename, QString channelName)
        : QDialog(parent), m_filename(filename), m_channelName(channelName)
{
    setupUi(this);
    setWindowTitle(tr("Change Icon for \"%1\" channel").arg(channelName));
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setScene( new QGraphicsScene(this) );
    QDir dir(":/channel");
    QFileInfoList list = dir.entryInfoList();
    int w = 80;
    int h = 50;
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QPixmap pix1 = QPixmap(":/channel/"+fileInfo.fileName()).scaledToHeight(h-10, Qt::SmoothTransformation);
        QPixmap pix = QPixmap(w, h);
        QPainter *paint = new QPainter( &pix );
        paint->fillRect(pix.rect(), QBrush(Qt::white));

        paint->drawPixmap((w-pix1.width())/2, 0, pix1);
        paint->setFont(QFont("Times", 8));
        QString title = fileInfo.fileName().section(".", 0, -2);
        paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
        delete paint;
        m_list << PairIcon(":/channel/"+fileInfo.fileName(), QPixmap(":/channel/"+fileInfo.fileName()));
    }
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("externalIcons");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString s = settings.value("filename").toString();
        QPixmap pix1 = QPixmap(s).scaledToHeight(h-10, Qt::SmoothTransformation);
        QPixmap pix = QPixmap(w, h);
        QPainter *paint = new QPainter( &pix );
        paint->fillRect(pix.rect(), QBrush(Qt::white));

        paint->drawPixmap((w-pix1.width())/2, 0, pix1);
        paint->setFont(QFont("Times", 8));
        QString title = s.section("/", -1).section(".", 0, -2);
        paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
        delete paint;
        ChannelIconItem *item = new ChannelIconItem(pix, s, m_filename==s, this);
        view->scene()->addItem( item );
        item->setPos(m_x, m_y);
        m_x += (w + 10);
        if ( m_x+w+10 >= 560 )
        {
            m_x = 0;
            m_y += (h + 10);
        }
    }
    settings.endArray();
}
//
void ChangeIconImpl::channelIconClicked(ChannelIconItem *item, bool doubleClick)
{
    QList<QGraphicsItem *> list = view->scene()->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        if ( *it )
        {
            ChannelIconItem *iconItem = (ChannelIconItem *)*it;
            iconItem->setSelected(*it == item);
        }
    }
    m_selectedFilename = item->filename();
    if ( doubleClick )
    {
        on_buttonBox_accepted();
        accept();
    }
}


void ChangeIconImpl::on_buttonBox_accepted()
{
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Options");
    settings.setValue("iconchannel-"+m_channelName, m_selectedFilename);
    settings.endGroup();
}


void ChangeIconImpl::on_filterEdit_textChanged(QString s)
{
    delete view->scene();
    view->setScene( new QGraphicsScene(this) );
    ChannelIconItem *activeItem = 0;
    m_x = m_y = 0;
    int w = 80;
    int h = 50;
    foreach(PairIcon pairIcon, m_list)
    {
        if ( pairIcon.icon().toLower().contains( s.toLower() ) )
        {
            QPixmap pix1 = pairIcon.pixmap().scaledToHeight(h-10, Qt::SmoothTransformation);
            QPixmap pix = QPixmap(w, h);
            QPainter *paint = new QPainter( &pix );
            paint->fillRect(pix.rect(), QBrush(Qt::white));

            paint->drawPixmap((w-pix1.width())/2, 0, pix1);
            paint->setFont(QFont("Times", 8));
            QString title = pairIcon.icon().section("/", -1).section(".", 0, -2);
            paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
            delete paint;
            ChannelIconItem *item = new ChannelIconItem(pix, pairIcon.icon(), m_filename==pairIcon.icon(), this);
            if ( m_filename==pairIcon.icon() )
            {
                activeItem = item;
            }
            view->scene()->addItem( item );
            item->setPos(m_x, m_y);
            m_x += (w + 10);
            if ( m_x+w+10 >= 560 )
            {
                m_x = 0;
                m_y += (h + 10);
            }

        }
    }
    if ( activeItem )
    {
        view->centerOn(activeItem);
    }
}

void ChangeIconImpl::on_addButton_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Image"),
                "",
                tr("Images Files (*.* *)"));
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    if ( QPixmap( s ).isNull() )
    {
        QMessageBox::warning(this, tr("Image File"), tr("The file is not a valid image."));
        return;
    }
    int w = 80;
    int h = 50;
    QPixmap pix1 = QPixmap(s).scaledToHeight(h-10, Qt::SmoothTransformation);
    QPixmap pix = QPixmap(w, h);
    QPainter *paint = new QPainter( &pix );
    paint->fillRect(pix.rect(), QBrush(Qt::white));

    paint->drawPixmap((w-pix1.width())/2, 0, pix1);
    paint->setFont(QFont("Times", 8));
    QString title = s.section("/", -1).section(".", 0, -2);
    paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
    delete paint;
    ChannelIconItem *item = new ChannelIconItem(pix, s, false, this);
    m_list << PairIcon(s, QPixmap(s));
    view->scene()->addItem( item );
    item->setPos(m_x, m_y);
    m_x += (w + 10);
    if ( m_x+w+10 >= 560 )
    {
        m_x = 0;
        m_y += (h + 10);
    }
    view->centerOn(item);
    QStringList list;
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("externalIcons");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        list << settings.value("filename").toString();
    }
    settings.endArray();
    list << s;
    settings.beginWriteArray("externalIcons");
    for (int i = 0; i < list.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("filename", list.at(i));
    }
    settings.endArray();

}

void ChangeIconImpl::deleteIcon(ChannelIconItem *item)
{
    m_selectedFilename = item->filename();
    QStringList list;
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("externalIcons");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        list << settings.value("filename").toString();
    }
    settings.endArray();
    list.removeAll(m_selectedFilename);
    settings.beginWriteArray("externalIcons");
    for (int i = 0; i < list.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("filename", list.at(i));
    }
    settings.endArray();
    delete item;
    if ( m_filename == m_selectedFilename )
    {
        m_filename = QString();
    }
}


void ChangeIconImpl::resizeEvent(QResizeEvent *event)
{
	QDialog::resizeEvent( event );
    delete view->scene();
    view->setScene( new QGraphicsScene(this) );
    view->setSceneRect(view->rect() );
    ChannelIconItem *activeItem = 0;
    m_x = m_y = 0;
    int w = 80;
    int h = 50;
    foreach(PairIcon pairIcon, m_list)
    {
        QPixmap pix1 = pairIcon.pixmap().scaledToHeight(h-10, Qt::SmoothTransformation);
        QPixmap pix = QPixmap(w, h);
        QPainter *paint = new QPainter( &pix );
        paint->fillRect(pix.rect(), QBrush(Qt::white));

        paint->drawPixmap((w-pix1.width())/2, 0, pix1);
        paint->setFont(QFont("Times", 8));
        QString title = pairIcon.icon().section("/", -1).section(".", 0, -2);
        paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
        delete paint;
        ChannelIconItem *item = new ChannelIconItem(pix, pairIcon.icon(), m_filename==pairIcon.icon(), this);
        if ( m_filename==pairIcon.icon() )
        {
            activeItem = item;
        }
        view->scene()->addItem( item );
        item->setPos(m_x, m_y);
        m_x += (w + 10);
        if ( m_x+w+10 >= view->sceneRect().width() )
        {
            m_x = 0;
            m_y += (h + 10);
        }
    }
    view->setSceneRect(view->scene()->itemsBoundingRect() );
    if ( activeItem )
    {
        view->centerOn(activeItem);
    }
}

