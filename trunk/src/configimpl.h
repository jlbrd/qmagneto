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

#ifndef CONFIGIMPL_H
#define CONFIGIMPL_H
//
#include <QDialog>
#include "ui_config.h"
#include <mainwindowimpl.h>
//
class ConfigImpl : public QDialog, public Ui::Config
{
Q_OBJECT
public:
	ConfigImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
  void on_format_activated(QString );
	void on_populateDB_clicked();
  void on_commandDirectory_clicked();
  void on_recordingDirectoryButton_clicked();
  void on_XmlFilenameButton_clicked();
private:
	MainWindowImpl *m_mainWindowImpl;
};
#endif





