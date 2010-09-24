#include "channeliconitem.h"
#include "changethumbimpl.h"
//
#include <QDebug>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ChannelIconItem::ChannelIconItem( QPixmap pixmap, QPixmap originalPixmap, QString filename, bool selected, QObject *parent)
        : QGraphicsPixmapItem(pixmap), m_pixmap(pixmap), m_filename(filename), m_selected(selected), m_originalPixmap(originalPixmap)
{
    m_parent = (ChangeThumbImpl *) parent;
    setSelected( m_selected );
}
void ChannelIconItem::setSelected(bool selected)
{
    if( selected ) QD<<m_originalPixmap.size();
    m_selected = selected;
    QPixmap pix = m_pixmap;
    int w = 3;
    if ( selected )
    {
        QPainter *paint = new QPainter( &pix );
        paint->setPen(QPen(QBrush(Qt::blue), w));
        paint->drawRoundedRect(
            pix.rect().x(),
            pix.rect().y(),
            pix.rect().width()-w,
            pix.rect().height()-w,
            5.0,
            3.0
        );
        delete paint;

    }
    setPixmap( pix );
}
void ChannelIconItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->buttons() == Qt::LeftButton )
    {
        emit channelIconClicked(this, false);
    }
    else if ( event->buttons() == Qt::RightButton && !m_filename.startsWith(":/channel/") && !m_filename.toLower().startsWith("http://"))
    {
        QMenu *menu = new QMenu(m_parent);
        connect(menu->addAction(
                    QIcon(""),
                    tr("Delete Icon")), SIGNAL(triggered()),
                this,
                SLOT(slotDeleteIcon())
               );
        menu->exec(event->screenPos());
        delete menu;
    }
}

void ChannelIconItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
    emit channelIconClicked(this, true);
}

void ChannelIconItem::slotDeleteIcon()
{
   emit deleteIcon( this );
}

