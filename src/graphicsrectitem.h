#ifndef GRAPHICSRECTITEM_H
#define GRAPHICSRECTITEM_H
//
#include <QGraphicsRectItem>
#include <QObject>
#include <QFont>
#include "defs.h"
class MainWindowImpl;
//
class GraphicsRectItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public slots:
	void slotImageAvailable(PairIcon pairIcon);
private:
	static QFont m_programFont;
	int m_posIn;
	bool m_enabled;
	bool m_inCurrentHour;
	MainWindowImpl *m_main;
public:
	static void setProgramFont(QFont value) { m_programFont = value; };
	static QFont programFont() { return m_programFont; };
	void setEnabled(bool value);
	void setInCurrentHour(bool value);
	enum Kind { Channel, Program, HourRect, Hour };
	GraphicsRectItem(MainWindowImpl *main, const QRectF & rect, const QString text, const Kind kind, const PairIcon pairIcon=PairIcon(), const int star=0);
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *widget=0);
private:
	QString m_text;	
	Kind m_kind;
	PairIcon m_pairIcon;
	int m_star;
protected:
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
	virtual void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event );
	virtual void mousePressEvent( QGraphicsSceneMouseEvent * event );
};
#endif
