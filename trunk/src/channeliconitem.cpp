#include "channeliconitem.h"
#include "changeiconimpl.h"
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ChannelIconItem::ChannelIconItem( QPixmap pixmap, QString filename, bool selected, QObject *parent)
        : QGraphicsPixmapItem(pixmap), m_pixmap(pixmap), m_filename(filename), m_selected(selected)
{
    m_parent = (ChangeIconImpl *) parent;
    setSelected( m_selected );
}
void ChannelIconItem::setSelected(bool selected)
{
    //QD<<selected;
    //if ( selected == m_selected )
        //return;
    m_selected = selected;
    QPixmap pix = m_pixmap;
    int w = 3;
    if ( selected )
    {
        QPainter *paint = new QPainter( &pix );
        paint->setPen(QPen(QBrush(Qt::black), w));
        paint->drawRect(
            pix.rect().x(),
            pix.rect().y(),
            pix.rect().width()-w,
            pix.rect().height()-w
        );
        delete paint;

    }
    setPixmap( pix );
}
void ChannelIconItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    m_parent->channelIconClicked(this, false);
}

void ChannelIconItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
    m_parent->channelIconClicked(this, true);
}
