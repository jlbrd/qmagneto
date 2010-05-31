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

#include "configimpl.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QInputDialog>
#include <QComboBox>
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
    connect(XmlFilename, SIGNAL(textChanged(QString)), fromFile, SLOT(click()));
}
//

void ConfigImpl::on_populateDB_clicked()
{
    if ( fromFile->isChecked() )
    {
        emit populateDB(0, XmlFilename->text() );
    }
    else if ( fromUrl->isChecked() )
    {
        emit populateDB(1, comboURL->currentText() );
    }
    else 
    {
        emit populateDB(2, customCommand->text() );
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
void ConfigImpl::on_addCategory_clicked()
{
     bool ok;
     QString text = QInputDialog::getText(this, tr("QMagneto"),
                                          tr("Category:"), QLineEdit::Normal,
                                          "", &ok);
     if (ok && !text.isEmpty())
         googleImageCategories->addItem(text);
}

void ConfigImpl::on_deleteCategory_clicked()
{
	QListWidgetItem *item = googleImageCategories->currentItem();
	if( item )
		delete item;
	
}


QStringList ConfigImpl::comboURLEntries()
{
    QStringList list;
    for(int i=0; i<comboURL->count(); i++)
    {
        list << comboURL->itemText( i );
    }
    return list;
}


void ConfigImpl::slotAddURL()
{
     bool ok;
     QString text = QInputDialog::getText(this, tr("QMagneto"),
                                          tr("URL:"), QLineEdit::Normal,
                                          "", &ok);
     if (ok && !text.isEmpty())
         m_uiEditURL.listWidget->addItem(text);
}


void ConfigImpl::slotRemoveURL()
{
	QListWidgetItem *item = m_uiEditURL.listWidget->currentItem();
	if( item )
		delete item;
}


void ConfigImpl::on_URLButton_clicked()
{
    QDialog dialog;
    m_uiEditURL.setupUi( &dialog );
    connect(m_uiEditURL.add, SIGNAL(clicked()), this, SLOT(slotAddURL()) );
    connect(m_uiEditURL.remove, SIGNAL(clicked()), this, SLOT(slotRemoveURL()) );
    m_uiEditURL.listWidget->addItems(comboURLEntries());
    m_uiEditURL.listWidget->setCurrentRow( 0 );
    if( dialog.exec() == QDialog::Accepted )
    {
        comboURL->clear();
        for(int i=0; i < m_uiEditURL.listWidget->count(); i++)
        {
            comboURL->addItem( m_uiEditURL.listWidget->item( i )->text() );
        }
        if( comboURL->count() == 0 )
        {
            fromFile->setChecked(true);
        }
    }
}
