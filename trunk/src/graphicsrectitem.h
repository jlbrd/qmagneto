/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2010  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qmagneto/
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
private slots:
	void slotDeleteIcon();
	void slotChangeChannelIcon();
public slots:
	void slotDeleteThumb();
	void slotShowAlertWhenStarts();
	void slotAddProgram();
	void slotChangeThumb();
	void slotShowIcon(int id, int kind, bool active);
    void slotImageAvailable(PairIcon pairIcon);
private:
	void showExpanded(QPainter *painter);
	void showNormal(QPainter *painter);
	bool m_expanded;
	bool m_showAlert;
	bool m_showReading;
	bool m_showRecording;
    static QFont m_programFont;
    int m_posIn;
    bool m_enabled;
    bool m_inCurrentHour;
    int m_id;
    QDateTime m_stop;
    QDateTime m_start;
    MainWindowImpl *m_main;
    QString m_channel;
    QRect m_posPixmap;
public:
	QString m_expandedText;
    static void setProgramFont(QFont value)
    {
        m_programFont = value;
    };
    static QFont programFont()
    {
        return m_programFont;
    };
    void setEnabled(bool value);
    void setInCurrentHour(bool value);
    enum Kind
    {
        Channel, Program, HourRect, Hour
    };
    GraphicsRectItem(MainWindowImpl *main, const int id, const QRectF & rect, 
    	const QDateTime start, const QDateTime stop, const QString text, 
    	const Kind kind, PairIcon pairIcon, const int star, const QString channel, const bool showDate);
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *widget=0);
    bool isEnabled()
    {
        return m_enabled;
    }
    int id()
    {
        return m_id;
    }
    QString text()
    {
        return m_text;
    }
    QDateTime stop()
    {
        return m_stop;
    }
    QDateTime start()
    {
        return m_start;
    }
    QString channel()
    {
        return m_channel;
    }
    Kind kind()
    {
        return m_kind;
    }
    PairIcon pairIcon()
    {
    	return m_pairIcon;
   	}
private:
    QString m_text;
    bool m_showDate;
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
