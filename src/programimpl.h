/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2009  Jean-Luc Biord
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
* Program URL   : http://code.google.com/p/qmagneto/
*
*/

#ifndef PROGRAMIMPL_H
#define PROGRAMIMPL_H
//
#include <QDialog>
#include "ui_program.h"
#include "mainwindowimpl.h"
#include "xmldefaulthandler.h"
//
class ProgramImpl : public QDialog, public Ui::Program
{
Q_OBJECT
public:
	ProgramImpl( QWidget * parent, TvProgram prog, QString formatNomFichier);
	MainWindowImpl::Kind kind() { return m_kind; };
	void setType(MainWindowImpl::Kind t) { m_kind = t; };
private slots:
	void on_directoryButton_clicked();
	void on_filename_cursorPositionChanged(int , int );
	void on_viewButton_clicked();
private:
	TvProgram m_prog;
	MainWindowImpl *m_mainWindowImpl;
	MainWindowImpl::Kind m_kind;
};
#endif





