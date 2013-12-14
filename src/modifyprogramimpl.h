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
#ifndef MODIFYPROGRAMIMPL_H
#define MODIFYPROGRAMIMPL_H
//
#include <QDialog>
#include "ui_modifyprogram.h"
#include "mainwindowimpl.h"
//
class QTableWidget;

class ModifyProgramImpl : public QDialog, public Ui::ModifyProgram
{
Q_OBJECT
public:
	ModifyProgramImpl( MainWindowImpl * parent, QTableWidget *table, int row );
private slots:
	void on_buttonBox_accepted();
	void on_directoryButton_clicked();
private:
	QTableWidget *m_table;
	MainWindowImpl * m_main;
	int m_row;
};
#endif





