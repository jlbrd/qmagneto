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

#include "programimpl.h"
#include <QFileDialog>
//
ProgramImpl::ProgramImpl( QWidget * parent,TvProgram prog, QString formatNomFichier)
        : QDialog(parent)
{
    setupUi(this);
    m_mainWindowImpl = (MainWindowImpl *)parent;
    directory->setText( m_mainWindowImpl->directory() );
    m_kind = MainWindowImpl::Recording;
    m_prog.channelName = prog.channelName;
    m_prog.title = prog.title;
    m_prog.start = prog.start;
    m_prog.before = prog.before;
    m_prog.after = prog.after;
    filename->setText( formatNomFichier );
    beginDate->setDate( prog.start.date() );
    beginHour->setTime( prog.start.time() );
    endDate->setDate( prog.stop.date() );
    endHour->setTime( prog.stop.time() );
    before->setValue( prog.before );
    after->setValue( prog.after );
    channel->setText( prog.channelName );
    programName->setTitle( prog.title );
    desc->clear();
    desc->setText( m_mainWindowImpl->showDescription( prog ) );

}
//

void ProgramImpl::on_filename_cursorPositionChanged(int , int )
{
    QString nouveauTitre = filename->text();
    nouveauTitre.replace("%n", m_prog.channelName);
    nouveauTitre.replace("%t", m_prog.title);
    nouveauTitre.replace("%y", m_prog.start.date().toString("yyyy"));
    nouveauTitre.replace("%m", m_prog.start.date().toString("MMM"));
    nouveauTitre.replace("%d", m_prog.start.date().toString("dd"));
    nouveauTitre.remove('/').remove('\\');
    overviewFilename->setText( directory->text() + nouveauTitre );
}

void ProgramImpl::on_viewButton_clicked()
{
    m_kind = MainWindowImpl::Reading;
    accept();
}

void ProgramImpl::on_directoryButton_clicked()
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
    if( !s.endsWith("/") )
    	s += "/";
    directory->setText( s );
    on_filename_cursorPositionChanged(0,0);
}
