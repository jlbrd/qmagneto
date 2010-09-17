#ifndef CHANNELICONITEM_H
#define CHANNELICONITEM_H
//
#include <QGraphicsPixmapItem>
#include <QObject>
//
class ChangeThumbImpl;
class ChannelIconItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
private slots:
    void slotDeleteIcon();
private:
	bool m_selected;
	QPixmap m_pixmap;
	QPixmap m_originalPixmap;
	ChangeThumbImpl *m_parent;
	QString m_filename;
public:
	bool selected() { return m_selected; }
	void setSelected(bool selected);
	QString filename() { return m_filename; }
    QPixmap pixmap() { return m_originalPixmap; }
	ChannelIconItem(QPixmap pixmap, QPixmap originalPixmap, QString filename, bool selected, QObject *parent);
protected:	
    virtual void mousePressEvent( QGraphicsSceneMouseEvent * event );
    virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
signals:
    void channelIconClicked(ChannelIconItem *, bool);
    void deleteIcon( ChannelIconItem *);
};
#endif
