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

#ifndef CANAUXIMPL_H
#define CANAUXIMPL_H
//
#include <QDialog>
#include "xmldefaulthandler.h"
#include "ui_channels.h"
//
class ChannelsImpl : public QDialog, public Ui::Channels
{
Q_OBJECT
public:
	ChannelsImpl( QWidget * parent, QList<TvChannel> channels );
private slots:
	void on_up_clicked();
	void on_down_clicked();
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
private:
};
#endif






