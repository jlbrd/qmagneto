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
#include "modifyprogramimpl.h"
#include <QFileDialog>
#include <QTableWidget>
#include <QTimer>
#include <QHeaderView>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ModifyProgramImpl::ModifyProgramImpl(MainWindowImpl * parent, QTableWidget *table, int row) 
	: QDialog(parent), m_table(table), m_row(row), m_main(parent)
{
	setupUi(this);
    QTableWidgetItem *item = m_table->item(m_row, 0);
    Program prog = item->data(Qt::UserRole).value<Program>();
    beginDate->setDateTime( prog.start );
    beginHour->setDateTime( prog.start );
    endDate->setDateTime( prog.end );
    endHour->setDateTime( prog.end );
    before->setValue( prog.before );
    after->setValue( prog.after );
    checkBoxBefore->setChecked( prog.before > 0 );
    checkBoxAfter->setChecked( prog.after > 0 );
    directory->setText( prog.directory );
    option->setText( prog.option );
    filename->setText( m_table->item(m_row, 3)->text().section(prog.directory, 1) );
	//QD << "entree" << prog.start << prog.before << prog.end << prog.after;
}
//
void ModifyProgramImpl::on_directoryButton_clicked()
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
}
//
void ModifyProgramImpl::on_buttonBox_accepted()
{
    QTableWidgetItem *item = m_table->item(m_row, 0);
    Program prog = item->data(Qt::UserRole).value<Program>();
	prog.start.setDate(beginDate->date());
	prog.start.setTime(beginHour->time());
	prog.end.setDate(endDate->date());
	prog.end.setTime(endHour->time());
	checkBoxBefore->isChecked() ? prog.before = before->value() : prog.before = 0;
	checkBoxAfter->isChecked() ? prog.after = after->value() : prog.after = 0;
	prog.directory = directory->text();
    prog.option = option->text();
	m_table->item(m_row, 1)->setText(prog.start.addSecs(prog.before*-60).toString(Qt::LocaleDate));
	m_table->item(m_row, 2)->setText(prog.end.addSecs(prog.after*60).toString(Qt::LocaleDate));
	m_table->item(m_row, 3)->setText(prog.directory+filename->text());
	int msecs = ( QDateTime::currentDateTime().secsTo( prog.start.addSecs(prog.before*-60) ) * 1000 );
	msecs = qMax(0, msecs);
	if( prog.timer == 0 )
	{
		prog.timer = new QTimer();
	    connect(prog.timer, SIGNAL(timeout()), (MainWindowImpl *)parent(), SLOT(slotTimer()));
	}
	prog.timer->start(msecs);
	//QD << "sortie " << prog.start << prog.before << prog.end << prog.after;
	QVariant v;
	v.setValue( prog );
	m_table->item(m_row, 0)->setData(Qt::UserRole, v );
    m_table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    m_main->saveRecording();

}
