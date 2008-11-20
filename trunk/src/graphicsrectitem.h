#ifndef GRAPHICSRECTITEM_H
#define GRAPHICSRECTITEM_H
//
#include <QGraphicsRectItem>
class MainWindowImpl;
//
class GraphicsRectItem : public QGraphicsRectItem
{
private:
	int m_posDedans;
	bool m_actif;
	bool m_dansHeureCourante;
	MainWindowImpl *m_main;
public:
	void setActif(bool value);
	void setDansHeureCourante(bool value);
	enum Type { Chaine, Programme, CadreHeure, Heure };
	GraphicsRectItem(MainWindowImpl *main, const QRectF & rect, const QString text, const Type type, const QPixmap pixmap=QPixmap(), const int star=0);
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *widget=0);
private:
	QString m_text;	
	Type m_type;
	QPixmap m_pixmap;
	int m_star;
protected:
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
	virtual void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event );
	virtual void mousePressEvent( QGraphicsSceneMouseEvent * event );
};
#endif
