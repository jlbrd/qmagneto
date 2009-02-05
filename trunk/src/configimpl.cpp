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

#include "configimpl.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

ConfigImpl::ConfigImpl( QWidget * parent, Qt::WFlags f)
        : QDialog(parent, f)
{
    setupUi(this);
    m_mainWindowImpl = (MainWindowImpl *) parent;
    if ( !QFile::exists(command->text() ) )
    {
        // TODO
    }
}
//

void ConfigImpl::on_populateDB_clicked()
{
    if ( fromFile->isChecked() )
    {
        m_mainWindowImpl->populateDB(true, XmlFilename->text() );
    }
    else
    {
        m_mainWindowImpl->populateDB(false, comboURL->currentText() );
    }
}

void ConfigImpl::on_recordingDirectoryButton_clicked()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the project directory"),
                    recordingDirectory->text(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    recordingDirectory->setText( s );
}
void ConfigImpl::on_XmlFilenameButton_clicked()
{
    QString s = QFileDialog::getOpenFileName(
                    this,
                    tr("XML Filename"),
                    XmlFilename->text(),
                    tr("XML Files (*.xml *.XML *)"));
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    XmlFilename->setText( s );
}
void ConfigImpl::on_commandDirectory_clicked()
{
    QString s = QFileDialog::getOpenFileName(
                    this,
                    tr("Choose the recording command"),
                    command->text(),
#ifdef Q_OS_WIN32
                    tr("Executable file (*.exe *.EXE)"));
#else
                    tr("*"));
#endif
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    command->setText( s );
}


void ConfigImpl::on_format_activated(QString s)
{
    if ( s == QObject::tr("Custom") )
    {
        recordingOptions->setVisible(true);
    }
    else
    {
        recordingOptions->setVisible(false);
    }
}
