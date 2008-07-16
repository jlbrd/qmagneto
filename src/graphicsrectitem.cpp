#include "graphicsrectitem.h"
#include "xmldefaulthandler.h"
#include "mainwindowimpl.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
GraphicsRectItem::GraphicsRectItem(MainWindowImpl *main,  const QRectF & rect, const QString text, const Type type)
        : m_main(main), QGraphicsRectItem(rect), m_text(text), m_type(type)
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
    QRectF r = rect();
    ProgrammeTV prog = data(0).value<ProgrammeTV>();
    if ( m_type == Chaine )
    {
        painter->setBrush(Qt::white);
        painter->setPen(Qt::white);
        painter->drawRect(r);
        if( !m_text.isEmpty() )
        {
	        QImage logo = QImage(":/images/images/"+m_text+".png").scaled(70, 74, Qt::KeepAspectRatio);
	        painter->drawImage(QRectF(
	                               r.x()+((r.width()-logo.width())/2.0),
	                               r.y()+((r.height()-logo.height())/2.0),
	                               logo.width(),
	                               logo.height()),
	                           logo);
       	}
        painter->setPen(Qt::black);
        painter->drawLine(r.width(), r.y(), r.width(), r.y()+r.height());
#ifdef RIEN
        if ( m_posDedans )
        {
            //70x54
            QImage play = QImage(":/images/images/play.png");
            if ( m_posDedans == 2 )
            {
                QRectF rectPlay = QRectF(
                                      rect().x()+((rect().width()-70)/2.0)+70-play.width(),
                                      rect().y()+2,
                                      play.width(),
                                      play.height()
                                  );
                painter->drawRect(rectPlay);
            }
            QPointF pointPlay = QPointF(
                                    r.x()+((r.width()-70)/2.0)+70-play.width(),
                                    r.y()+2
                                );
            painter->drawImage(pointPlay, play);
        }
#endif
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
        painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeft, t);

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

void GraphicsRectItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if ( m_type == Chaine )
    {
    }
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

void GraphicsRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
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

void GraphicsRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    m_posDedans = 0;
    update();
}

