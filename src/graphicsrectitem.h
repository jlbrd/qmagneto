/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2009  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify.
*  But to reuse the source code, permission of the author is essential. Without authorization, code reuse is prohibited.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://code.google.com/p/qmagneto/
*
*/

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
	GraphicsRectItem(MainWindowImpl *main, const QRectF & rect, const QString text, const Kind kind, PairIcon pairIcon=PairIcon(), const int star=0);
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *widget=0);
  bool isEnabled() { return m_enabled; }
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
