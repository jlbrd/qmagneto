#include "graphicsrectitem.h"
#include "xmldefaulthandler.h"
#include "mainwindowimpl.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
extern QGraphicsView *viewP;
QFont GraphicsRectItem::m_programFont = QFont();

GraphicsRectItem::GraphicsRectItem(MainWindowImpl *main, const QRectF & rect, const QString text, const Kind kind, const QPixmap pixmap, const int star)
        : QGraphicsRectItem(rect), m_main(main), m_text(text), m_kind(kind), m_pixmap(pixmap), m_star(star)
{
    m_inCurrentHour = false;
    m_enabled = false;
    m_posIn = 0;
    if ( kind == Channel )
    {
        setAcceptsHoverEvents( true );
    }
}
//

void GraphicsRectItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * , QWidget *)
{
    TvProgram prog = data(0).value<TvProgram>();
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
        return;
    }
    painter->setFont(m_programFont);
    painter->setClipRect(r);
    painter->setClipping(true);
    if ( m_kind == Channel )
    {
        painter->setBrush(Qt::white);
        painter->setPen(Qt::white);
        painter->drawRect(r);
        if ( !m_text.isEmpty() )
        {
            QPixmap logo = m_pixmap;
            if ( logo.height() > r.height()-10 )
                logo = logo.scaledToHeight( r.height()-10 );
            painter->setPen(Qt::black);
            if ( m_pixmap.isNull() )
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
        r.adjust(0, 5, 0, -5);
        if ( m_enabled )
            painter->setBrush(QColor(Qt::green).lighter());
        else if ( m_inCurrentHour )
            painter->setBrush(QColor(Qt::blue).lighter());
        else
            painter->setBrush(Qt::white);
        painter->drawRect(r);
        QString t = prog.title+"\n"+prog.start.toString("hh:mm")+"-"+prog.stop.toString("hh:mm");
        r.adjust(2, 2, 0, 0);
        QRectF r2 = painter->fontMetrics().boundingRect(QRect(r.x(),r.y(),r.width(),r.height()),
        	Qt::AlignTop | Qt::AlignLeft,
        	t
        );
        painter->drawText(r2, Qt::AlignBottom | Qt::AlignLeft, t);
        r2.setX(r2.x()+painter->fontMetrics().boundingRect(prog.start.toString("hh:mm")+"-"+prog.stop.toString("hh:mm")).width());
        r2.setY(r2.y()+r2.height() - painter->fontMetrics().boundingRect(prog.start.toString("hh:mm")+"-"+prog.stop.toString("hh:mm")).height());
        if ( m_star )
        {
            QPixmap pixmap = QPixmap(":/images/images/star.png").scaledToHeight( 12 );
            int i;
            for (i=0; i<m_star; i++)
                painter->drawPixmap(
                    r2.x() + ((i+1)*pixmap.width()+2),
                    r2.y() + ( r2.height()/2 - pixmap.height()/2 ),
                    pixmap);
        }
        if ( !m_pixmap.isNull() )
        {
            QPixmap pixmap = m_pixmap.scaledToHeight( r.height()-4 );
            painter->drawPixmap(r.x()+r.width()-pixmap.width(), r.y()+2, pixmap);

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

void GraphicsRectItem::mousePressEvent( QGraphicsSceneMouseEvent * )
{
    if ( m_kind == Channel )
    {}
    else
    {
        m_main->slotItemClicked(this);
    }
}

void GraphicsRectItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * )
{
    TvProgram prog = data(0).value<TvProgram>();
    if ( !prog.start.isValid() )
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
#ifdef RIEN
    QImage play = QImage(":/images/images/play.png");
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
    m_posIn = 0;
    update();
}

void GraphicsRectItem::setInCurrentHour(bool value)
{
    m_inCurrentHour = value;
}
