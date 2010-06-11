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

#ifndef CHANGEICONIMPL_H
#define CHANGEICONIMPL_H
//
#include <QDialog>
#include "defs.h"
#include "ui_changeicon.h"
//

class ChannelIconItem;
class ChangeIconImpl : public QDialog, public Ui::ChangeIcon
{
Q_OBJECT
protected:
	virtual void resizeEvent(QResizeEvent *);
public:
	ChangeIconImpl( QWidget * parent, QString filename, QString channelName );
public slots:
	void deleteIcon(ChannelIconItem *item);
	void channelIconClicked(ChannelIconItem *item, bool doubleClick);
private slots:
	void on_addButton_clicked();
	void on_filterEdit_textChanged(QString );
	void on_buttonBox_accepted();
private:
	QList<PairIcon> m_list;
	QString m_filename;
	QString m_selectedFilename;
	QString m_channelName;
	int m_x;
	int m_y;
};
#endif





