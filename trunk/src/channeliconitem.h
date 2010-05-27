#ifndef CHANNELICONITEM_H
#define CHANNELICONITEM_H
//
#include <QGraphicsPixmapItem>
//
class ChangeIconImpl;
class ChannelIconItem : public QGraphicsPixmapItem
{
private:
	bool m_selected;
	QPixmap m_pixmap;
	ChangeIconImpl *m_parent;
	QString m_filename;
public:
	bool selected() { return m_selected; }
	void setSelected(bool selected);
	QString filename() { return m_filename; }
	ChannelIconItem(QPixmap pixmap, QString filename, bool selected, QObject *parent);
protected:	
    virtual void mousePressEvent( QGraphicsSceneMouseEvent * event );
    virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
};
#endif
