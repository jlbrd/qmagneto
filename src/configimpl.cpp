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
    connect(directoryButton, SIGNAL(clicked()), this, SLOT(slotDirectory()) );
    connect(XmlFilenameButton, SIGNAL(clicked()), this, SLOT(slotXml()) );
}
//

void ConfigImpl::on_populateDB_clicked()
{
    if ( fromFile->isChecked() )
    {
        //m_mainWindowImpl->populateDB(true, XmlFilename->text() );
        emit populateDB(true, XmlFilename->text() );
    }
    else
    {
        //m_mainWindowImpl->populateDB(false, comboURL->currentText() );
        emit populateDB(false, comboURL->currentText() );
    }
}

void ConfigImpl::slotDirectory()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the project directory"),
                    directory->text(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    directory->setText( s );
}
void ConfigImpl::slotXml()
{
    QString s = QFileDialog::getOpenFileName(this, tr("XML Filename"),
                XmlFilename->text(),
                tr("XML Files (*.xml *.XML *)"));
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    XmlFilename->setText( s );
}
