#include "expandedpixmap.h"
#include "xmldefaulthandler.h"
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ExpandedPixmap::ExpandedPixmap( const QPixmap & pixmap, XmlDefaultHandler *handler ) 
	: QGraphicsPixmapItem(pixmap), m_handler(handler)
{
	// TODO
}
//

void ExpandedPixmap::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
	m_handler->hideExpandedPixmap(this);
}

