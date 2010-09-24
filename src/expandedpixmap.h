#ifndef EXPANDEDPIXMAP_H
#define EXPANDEDPIXMAP_H
//
#include <QGraphicsPixmapItem>
class XmlDefaultHandler;
//
class ExpandedPixmap :public QGraphicsPixmapItem
{
protected:
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
public:
	ExpandedPixmap(const QPixmap & pixmap, XmlDefaultHandler *handler);
private:
	XmlDefaultHandler *m_handler;
};
#endif
