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

GraphicsRectItem::GraphicsRectItem(MainWindowImpl *main,  const QRectF & rect, const QString text, const Type type, const QPixmap pixmap, const int star)
        : QGraphicsRectItem(rect), m_main(main), m_text(text), m_type(type), m_pixmap(pixmap), m_star(star)
{
    m_dansHeureCourante = false;
    m_actif = false;
    m_posDedans = 0;
    if ( type == Chaine )
    {
        setAcceptsHoverEvents( true );
    }
}
//

void GraphicsRectItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * , QWidget *)
{
    ProgrammeTV prog = data(0).value<ProgrammeTV>();
    //QD << viewP->viewport()->rect() << r.toRect() <<
    //viewP->horizontalScrollBar()->value() << m_text;
    QRectF r = rect();
    QRect viewport = viewP->viewport()->rect();
    //QRect v = viewport.adjusted(viewP->horizontalScrollBar()->value(), viewP->verticalScrollBar()->value(), 0, 0);
    QRect v(viewport.x()+viewP->horizontalScrollBar()->value(),
            viewport.x()+viewP->verticalScrollBar()->value(),
            viewport.x()+viewP->horizontalScrollBar()->value()+viewport.width(),
            viewport.x()+viewP->verticalScrollBar()->value()+viewport.height());

    QPoint p1(r.x(), r.y());
    QPoint p2(r.x()+r.width(), r.y()+r.height());
    if ( m_type == Programme && !v.contains(p1) && !v.contains(p2) )
    {
        //QD << v << p1 << p2;
        return;
    }
    painter->setClipRect(r);
    painter->setClipping(true);
    if ( m_type == Chaine )
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
    else if ( m_type == Programme )
    {
        r.adjust(0, 5, 0, -5);
        if ( m_actif )
            painter->setBrush(QColor(Qt::green).lighter());
        else if ( m_dansHeureCourante )
            painter->setBrush(QColor(Qt::blue).lighter());
        else
            painter->setBrush(Qt::white);
        painter->drawRect(r);
        QString t = prog.title+"\n"+prog.start.toString("hh:mm")+"-"+prog.stop.toString("hh:mm");
        r.adjust(2, 0, -2, 0);
        QRectF r2(r.x(), r.y(),
                  (qreal)painter->fontMetrics().boundingRect( t ).width(),
                  (qreal)painter->fontMetrics().boundingRect( t ).height()
                 );
        painter->drawText(r2, Qt::AlignBottom | Qt::AlignLeft, t);
        r2 = QRectF(r.x() + (qreal)painter->fontMetrics().boundingRect( t.section("\n", 1, 1) ).width(),
                    r.y() + (qreal)painter->fontMetrics().boundingRect( t ).height()
                    -(qreal)painter->fontMetrics().boundingRect( t.section("\n", 1, 1) ).height(),
                    (qreal)painter->fontMetrics().boundingRect( t.section("\n", 1, 1) ).width(),
                    (qreal)painter->fontMetrics().boundingRect( t.section("\n", 1, 1) ).height()
                   );
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
    else if ( m_type == CadreHeure )
    {
        painter->setBrush(QColor(Qt::yellow).lighter());
        painter->drawRect(r);
    }
    else if ( m_type == Heure )
    {
        painter->drawText(r, Qt::AlignBottom | Qt::AlignHCenter, m_text);

    }

}

void GraphicsRectItem::mousePressEvent( QGraphicsSceneMouseEvent * )
{
    if ( m_type == Chaine )
    {}
    else
    {
        m_main->itemClique(this);
    }
}

void GraphicsRectItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * )
{
    ProgrammeTV prog = data(0).value<ProgrammeTV>();
    if ( !prog.start.isValid() )
        return;
    //QString resume;
    //if( prog.resume.count() )
    //resume = prog.resume.first();
    //m_main->ajouterProgramme(prog.channelName, prog.channel, prog.start, prog.stop, prog.title, desc);
    m_main->ajouterProgramme(prog);
}

void GraphicsRectItem::setActif(bool value)
{
    if ( m_actif == value )
        return;
    m_actif = value;
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
        m_posDedans = 2;
    }
    else
    {
        m_posDedans = 1;
    }
    update();
#endif
}

void GraphicsRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * )
{
    m_posDedans = 0;
    update();
}

void GraphicsRectItem::setDansHeureCourante(bool value)
{
    m_dansHeureCourante = value;
}
