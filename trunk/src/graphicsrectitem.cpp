#include "graphicsrectitem.h"
#include "xmldefaulthandler.h"
#include "mainwindowimpl.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
GraphicsRectItem::GraphicsRectItem(MainWindowImpl *main,  const QRectF & rect, const QString text, const Type type, const QPixmap pixmap, const int star)
        : m_main(main), QGraphicsRectItem(rect), m_text(text), m_type(type), m_pixmap(pixmap), m_star(star)
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
    painter->setClipRect(r);
    painter->setClipping(true);
    ProgrammeTV prog = data(0).value<ProgrammeTV>();
    if ( m_type == Chaine )
    {
        painter->setBrush(Qt::white);
        painter->setPen(Qt::white);
        painter->drawRect(r);
        if( !m_text.isEmpty() )
        {
	        //QImage logo = QImage(":/images/images/"+m_text+".png").scaled(70, 74, Qt::KeepAspectRatio);
			//QImage logo = m_pixmap.toImage().scaled(70, 74, Qt::KeepAspectRatio);
	        /*painter->drawImage(
	        					QRectF(
	                               r.x()+((r.width()-logo.width())/2.0),
	                               r.y()+((r.height()-logo.height())/2.0),
	                               logo.width(),
	                               logo.height()),
	                           logo);*/
			QPixmap logo = m_pixmap;
			if( logo.height() > r.height()-10 )
				logo = logo.scaledToHeight( r.height()-10 );
			painter->drawPixmap(
	        					
	                               r.x()+((r.width()-logo.width())/2.0),
	                               r.y()+((r.height()-logo.height())/2.0),
									logo);
       	}
        painter->setPen(Qt::black);
        painter->drawLine(r.width()-1, r.y(), r.width()-1, r.y()+r.height());
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
		if( m_star )
		{
			QPixmap pixmap = QPixmap(":/images/images/star.png").scaledToHeight( 12 );
			for(int i=0; i<m_star; i++)
				painter->drawPixmap(r.x()+70+(i*pixmap.width()+2), r.y()+26, pixmap);
			
		}
		if( !m_pixmap.isNull() )
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

