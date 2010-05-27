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

#include "changeiconimpl.h"
#include "graphicsrectitem.h"
#include "xmldefaulthandler.h"
#include "mainwindowimpl.h"
#include "defs.h"
#include <QSettings>
#include <QPainter>
#include <QString>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
extern QGraphicsView *viewP;
QFont GraphicsRectItem::m_programFont = QFont();

GraphicsRectItem::GraphicsRectItem(MainWindowImpl *main, const int id, const QRectF & rect,
                                   const QDateTime start, const QDateTime stop, const QString text,
                                   const Kind kind, PairIcon pairIcon, const int star, const QString channel, const bool showDate)
        : QObject(), m_id(id), QGraphicsRectItem(rect), m_start(start),
        m_stop(stop), m_main(main), m_text(text), m_kind(kind), m_star(star), m_channel(channel), m_showDate(showDate)
{
    m_pairIcon = PairIcon(pairIcon.icon(), pairIcon.pixmap());
    m_inCurrentHour = false;
    m_enabled = false;
    m_showAlert = false;
    m_showReading = false;
    m_showRecording = false;
    m_expanded = false;
    m_posIn = 0;
    if ( kind == Channel || kind == Program )
    {
        setAcceptsHoverEvents( true );
    }
    QObject::connect(
        m_main,
        SIGNAL(showIcon(int, int, bool)),
        this,
        SLOT(slotShowIcon(int, int,  bool))
    );
}
//

void GraphicsRectItem::showNormal(QPainter *painter)
{
    QRectF r = rect();
    QRect viewport = viewP->viewport()->rect();
    QRect v(viewport.x()+viewP->horizontalScrollBar()->value(),
            viewport.x()+viewP->verticalScrollBar()->value(),
            viewport.x()+viewP->horizontalScrollBar()->value()+viewport.width(),
            viewport.x()+viewP->verticalScrollBar()->value()+viewport.height());

    QPoint p1(r.x(), r.y());
    QPoint p2(r.x()+r.width(), r.y()+r.height());
    if ( m_kind == Program && !v.contains(p1) && !v.contains(p2) )
    {
        //QD << v << p1 << p2;
        //return;
    }
    painter->setFont(m_programFont);
    painter->setClipRect(r);
    painter->setClipping(true);
    if ( m_kind == Channel )
    {
        //painter->setBrush(Qt::white);
        if ( m_enabled )
            painter->setBrush(QColor(Qt::green).lighter());
        else
            painter->setBrush(Qt::white);
        painter->setPen(Qt::white);
        painter->drawRect(r);
        if ( !m_text.isEmpty() )
        {
            QPixmap logo = m_pairIcon.pixmap();
            if ( logo.height() > r.height()-10 )
                logo = logo.scaledToHeight( r.height()-10, Qt::SmoothTransformation );
            painter->setPen(Qt::black);
            if ( m_pairIcon.pixmap().isNull() )
            {
                painter->drawText(
                    rect(), Qt::AlignCenter,
                    m_text);

            }
            painter->drawPixmap(
                r.x()+((r.width()-logo.width())/2.0),
                r.y()+((r.height()-logo.height())/2.0),
                logo);
        }
        painter->setPen(Qt::black);
        painter->drawLine(r.width()-1, r.y(), r.width()-1, r.y()+r.height());
    }
    else if ( m_kind == Program )
    {
        setZValue(15);
        r.adjust(0, 5, 0, -5);
        if ( m_enabled )
            painter->setBrush(QColor(Qt::green).lighter());
        else if ( m_inCurrentHour )
            painter->setBrush(QColor(Qt::blue).lighter());
        else
            painter->setBrush(Qt::white);
        painter->drawRect(r);
        if ( m_showAlert )
        {
            QPixmap pixmap = QPixmap(":/images/bell.png");//.scaledToHeight( 12, Qt::SmoothTransformation);
            painter->drawPixmap(
                r.x() + 2,
                r.y() + ( r.height() - 14 ),
                12,
                12,
                pixmap);
        }
        if ( m_showReading )
        {
            QPixmap pixmap = QPixmap(":/images/play.png");//.scaledToHeight( 12, Qt::SmoothTransformation);
            painter->drawPixmap(
                r.x() + 15,
                r.y() + ( r.height() - 14 ),
                pixmap);
        }
        if ( m_showRecording )
        {
            QPixmap pixmap = QPixmap(":/images/save.png");//.scaledToHeight( 12, Qt::SmoothTransformation);
            painter->drawPixmap(
                r.x() + 28,
                r.y() + ( r.height() - 14 ),
                pixmap);
        }
        QString dateTime;
        if ( m_showDate )
            dateTime = m_start.date().toString("ddd dd MMMM") + ' ';
        dateTime += m_start.toString("hh:mm")+"-"+m_stop.toString("hh:mm");
        QString t = m_text+"\n"+dateTime;
        r.adjust(2, 2, 0, 0);
        QRectF r2 = painter->fontMetrics().boundingRect(QRect(r.x(),r.y(),r.width(),r.height()),
                    Qt::AlignTop | Qt::AlignLeft,
                    t
                                                       );
        painter->drawText(r2, Qt::AlignBottom | Qt::AlignLeft, t);
        r2.setX(r2.x()+painter->fontMetrics().boundingRect(dateTime).width());
        r2.setY(r2.y()+r2.height() - painter->fontMetrics().boundingRect(dateTime).height());
        if ( m_star )
        {
            QPixmap pixmap = QPixmap(":/images/star.png").scaledToHeight( 12, Qt::SmoothTransformation);
            int i;
            for (i=0; i<m_star; i++)
                painter->drawPixmap(
                    r2.x() + ((i+1)*pixmap.width()+2),
                    r2.y() + ( r2.height()/2 - pixmap.height()/2 ),
                    pixmap);
        }
        if ( !m_pairIcon.pixmap().isNull() && r.width()/2.5 > r.height() )
        {
            QPixmap pixmap = m_pairIcon.pixmap().scaledToHeight( r.height()-4, Qt::SmoothTransformation);
            painter->drawPixmap(r.x()+r.width()-pixmap.width()-1, r.y()+2, pixmap);

        }
        if ( m_main->showMode() == MainWindowImpl::Channel )
        {
            painter->setPen(Qt::black);
            painter->setBrush(Qt::transparent);
            painter->drawRect( rect().adjusted(0, 5, -1, -5) );
        }
    }
    else if ( m_kind == HourRect )
    {
        painter->setBrush(QColor(Qt::yellow).lighter());
        painter->drawRect(r);
    }
    else if ( m_kind == Hour )
    {
        painter->drawText(r, Qt::AlignBottom | Qt::AlignHCenter, m_text);

    }

}

void GraphicsRectItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->buttons() == Qt::LeftButton )
    {
        if ( m_kind == Channel )
        {
            m_main->slotItemClicked(this, -1);
        }
        else
        {
            m_main->slotItemClicked(this, m_id);
        }

    }
    else if ( event->buttons() == Qt::RightButton )
    {

        if ( m_kind == Channel )
        {
            QMenu *menu = new QMenu(m_main);
            connect(menu->addAction(
                        QIcon(""),
                        tr("Change Icon...")), SIGNAL(triggered()),
                    this,
                    SLOT(slotChangeChannelIcon())
                   );
            connect(menu->addAction(
                        QIcon(""),
                        tr("Delete Icon")), SIGNAL(triggered()),
                    this,
                    SLOT(slotDeleteIcon())
                   );
            menu->exec(event->screenPos());
            delete menu;
        }
        else
        {
            QMenu *menu = new QMenu(m_main);
            connect(menu->addAction(
                        QIcon(":/images/bell.png"),
                        tr("Show alert when starts")), SIGNAL(triggered()),
                    this,
                    SLOT(slotShowAlertWhenStarts())
                   );
            connect(menu->addAction(
                        QIcon(""),
                        tr("Add Program...")), SIGNAL(triggered()),
                    this,
                    SLOT(slotAddProgram())
                   );
            menu->exec(event->screenPos());
            delete menu;
        }
    }
}

void GraphicsRectItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * )
{
    TvProgram prog = m_main->handler()->tvProgram(m_id);
    if ( !m_start.isValid() )
        return;
    m_main->addProgram(prog);
}

void GraphicsRectItem::setEnabled(bool value)
{
    if ( m_enabled == value )
        return;
    m_enabled = value;
    update();
}

void GraphicsRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
    hoverEnterEvent( event );
}

void GraphicsRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent * )
{
    if ( m_kind == Channel )
    {
        setEnabled( true );
    }
    else if ( m_kind == Program && !m_expanded && (m_main->showMode() == MainWindowImpl::Channel || scene()->views().first()!= m_main->view() ))
    {
        m_expanded = true;
        m_main->handler()->expandItem(this, true);
    }
#ifdef RIEN
    QImage play = QImage(":/images/play.png");
    QRectF rectPlay = QRectF(
                          rect().x()+((rect().width()-70)/2.0)+70-play.width(),
                          rect().y()+2,
                          play.width(),
                          play.height()
                      );
    if ( rectPlay.contains(event->pos()) )
    {
        m_posIn = 2;
    }
    else
    {
        m_posIn = 1;
    }
    update();
#endif
}

void GraphicsRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * )
{
    if ( m_kind == Channel )
    {
        setEnabled( false );
    }
    else if ( m_kind == Program && m_expanded && (m_main->showMode() == MainWindowImpl::Channel || scene()->views().first()!= m_main->view() ))
    {
        m_expanded = false;
        m_main->handler()->expandItem(this, false);
    }
    m_posIn = 0;
    //update();
}

void GraphicsRectItem::setInCurrentHour(bool value)
{
    m_inCurrentHour = value;
}

void GraphicsRectItem::slotImageAvailable(PairIcon pairIcon)
{
    if ( pairIcon.icon() != m_pairIcon.icon() )
        return;
    m_pairIcon = PairIcon(m_pairIcon.icon(), pairIcon.pixmap());
    update();
}

void GraphicsRectItem::slotShowAlertWhenStarts()
{
    m_main->showAlertWhenStarts( m_id, !m_showAlert);
}
void GraphicsRectItem::slotAddProgram()
{
    TvProgram prog = m_main->handler()->tvProgram(m_id);
    if ( !m_start.isValid() )
        return;
    m_main->addProgram(prog);
}

void GraphicsRectItem::slotShowIcon(int id, int kind, bool active)
{
    if ( id != m_id )
        return;
    MainWindowImpl::Kind k = (MainWindowImpl::Kind)kind;
    switch ( k )
    {
    case MainWindowImpl::Alert:
        m_showAlert = active;
        break;
    case MainWindowImpl::Reading:
        m_showReading = active;
        break;
    case MainWindowImpl::Recording:
        m_showRecording = active;
        break;
    }
    update();
}


void GraphicsRectItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * , QWidget *)
{
    if ( m_expanded )
        showExpanded( painter );
    else
        showNormal( painter );
}


void GraphicsRectItem::showExpanded(QPainter *painter)
{
    setZValue(16);
    TvProgram prog = m_main->handler()->tvProgram(m_id);
    QRectF r = rect();
    painter->setFont(m_programFont);
    painter->setClipRect(r);
    painter->setClipping(true);
    r.adjust(0, 5, 0, -5);
    if ( m_enabled )
        painter->setBrush(QColor(Qt::green).lighter());
    else if ( m_inCurrentHour )
        painter->setBrush(QColor(Qt::blue).lighter());
    else
        painter->setBrush(Qt::white);
    painter->drawRect(r);
    QTextEdit *doc = new QTextEdit();
    doc->setGeometry(0,0, r.width(), 600);
    doc->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    doc->setText(m_main->showDescription(prog, true));
    QPixmap pix = QPixmap::grabWidget(doc);
    delete doc;
    QImage img = pix.toImage();
    for (int y=img.height()-30; y>0; y--)
    {
        if ( img.pixel(50, y) != QColor(Qt::white).rgb() )
        {
            img = img.copy(0,0,rect().width(), y-1);
            break;
        }
    }
    pix = QPixmap::fromImage(img.scaled(img.width()+15, img.height()+5));
    if ( scene()->views().first() == m_main->view() )
        pix.fill(QColor(Qt::red).light(188));
    else
        pix.fill(QColor(Qt::green).light(188));
    QPainter *paint = new QPainter( &pix );
    paint->fillRect(
        pix.rect().x()+5,
        pix.rect().y()+5,
        pix.rect().width()-5,
        pix.rect().height()-5,
        QBrush(Qt::darkGray));
    paint->setBrush(Qt::transparent);
    paint->drawImage(
        QPoint(0, 0),
        img,
        QRectF(img.rect().x(), img.rect().y(), img.rect().width()-5, img.rect().height()-5)
    );
    paint->drawRect(QRectF(img.rect().x()+1, img.rect().y()+1, img.rect().width()-5, img.rect().height()-5));
    delete paint;
    painter->drawPixmap(r.x(), r.y(), pix);
    setRect(rect().x(), rect().y(), rect().width(), pix.height());
    m_main->handler()->setPositionOnChannelMode(scene()->views().first());
    return;

}


void GraphicsRectItem::slotChangeChannelIcon()
{
	ChangeIconImpl changeIconImpl(m_main, XmlDefaultHandler::channelIconName(m_channel), m_channel);
	if( changeIconImpl.exec() == QDialog::Accepted )
	{
	    m_pairIcon = PairIcon(m_channel, QPixmap(XmlDefaultHandler::channelIconName(m_channel)));
		update();
	}
}


void GraphicsRectItem::slotDeleteIcon()
{
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Options");
    settings.setValue("iconchannel-"+m_channel, "");
    settings.endGroup();
	m_pairIcon = PairIcon(m_channel, QPixmap());
	update();
}

