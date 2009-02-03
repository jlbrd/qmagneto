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
	void on_filename_cursorPositionChanged(int , int );
	void on_viewButton_clicked();
private:
	TvProgram m_prog;
	MainWindowImpl *m_mainWindowImpl;
	MainWindowImpl::Kind m_kind;
};
#endif





