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

#include "programimpl.h"
//
ProgramImpl::ProgramImpl( QWidget * parent,TvProgram prog, QString formatNomFichier)
        : QDialog(parent)
{
    setupUi(this);
    m_mainWindowImpl = (MainWindowImpl *)parent;
    m_kind = MainWindowImpl::Recording;
    m_prog.channelName = prog.channelName;
    m_prog.title = prog.title;
    m_prog.start = prog.start;
    filename->setText( formatNomFichier );
    beginDate->setDate( prog.start.date() );
    beginHour->setTime( prog.start.time() );
    endDate->setDate( prog.stop.date() );
    endHour->setTime( prog.stop.time() );
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
    overviewFilename->setText( m_mainWindowImpl->directory() + nouveauTitre );
}

void ProgramImpl::on_viewButton_clicked()
{
    m_kind = MainWindowImpl::Reading;
    accept();
}
