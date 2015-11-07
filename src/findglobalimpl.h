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

#ifndef FINDGLOBALIMPL_H
#define FINDGLOBALIMPL_H
//
#include <QDialog>
#include "ui_findglobal.h"
//
class XmlDefaultHandler;
class MainWindowImpl;
class GraphicsRectItem;

class FindGlobalImpl : public QDialog, public Ui::FindGlobal
{
Q_OBJECT
public:
	void setCategories(QStringList value);
	FindGlobalImpl( QWidget * parent, XmlDefaultHandler *handler, Qt::WindowFlags f = 0 );
    QList<GraphicsRectItem *> listItemProgrammes()
    {
        return m_programsItemsList;
    };
private slots:
	void on_findButton_clicked();
	void slotHours(int value);
	void slotHours(bool value);
private:
    MainWindowImpl *m_main;
	QStringList m_categories;
	XmlDefaultHandler *m_handler;
    QList<GraphicsRectItem *> m_programsItemsList;

protected:
	void resizeEvent(QResizeEvent * event);
};
#endif





