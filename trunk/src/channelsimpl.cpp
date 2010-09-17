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

#include "channelsimpl.h"
#include "changethumbimpl.h"
#include "mainwindowimpl.h"
#include <QDir>
#include <QHeaderView>
#include <QSettings>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
//
ChannelsImpl::ChannelsImpl( QWidget * parent,QList<TvChannel> channels, XmlDefaultHandler *handler)
        : QDialog(parent), m_handler(handler)
{
    setupUi(this);
    m_main = (MainWindowImpl *)parent;
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    QHeaderView *header = table->horizontalHeader();
    header->resizeSection( 0, 160 );
    header->resizeSection( 1, 160 );
    header->resizeSection( 2, 380 );
    table->verticalHeader()->hide();
    int row = 0;
    foreach(TvChannel channel, channels)
    {
        QTableWidgetItem *newItem;
        table->setRowCount(row+1);
        if( channel.name.contains(QChar(255)) ) {
        	channel.name = channel.name.replace(QChar(255), ' ');
       	}
        
        newItem = new QTableWidgetItem(QIcon(m_handler->pairIcon(channel.id).pixmap()), channel.name.at(0).toUpper()+channel.name.mid(1) );
        newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        newItem->setCheckState( (Qt::CheckState)settings.value(channel.id+"-isEnabled", Qt::Checked).toInt() );
        table->setItem(row, 0, newItem);

        newItem = new QTableWidgetItem(channel.id);
        newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        table->setItem(row, 1, newItem);

        table->setItem(row++, 2, new QTableWidgetItem( settings.value(channel.id, "rtsp://mafreebox.freebox.fr/fbxtv_pub/stream?namespace=1&service=NONE&flavour=sd").toString() ));
    }
    settings.endGroup();
}
//

void ChannelsImpl::on_buttonBox_accepted()
{
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    for (int row=0; row<table->rowCount(); row++ )
    {
        QTableWidgetItem *item = table->item(row, 1);
        QTableWidgetItem *item2 = table->item(row, 2);
        settings.setValue(item->text(), item2->text());
        settings.setValue(item->text()+"-isEnabled", table->item(row, 0)->checkState());
        m_handler->setEnableChannel(item->text(), table->item(row, 0)->checkState()==2);
        settings.setValue("pos"+QString::number(row), item->text());
    }
    settings.endGroup();
    accept();
}

void ChannelsImpl::on_buttonBox_rejected()
{
    close();
}


void ChannelsImpl::on_up_clicked()
{
    int row = table->currentRow();
    QD << row;
    if ( row == 0 || row == -1 )
        return;
    QString s0 = table->item(row, 0)->text();
    QString s1 = table->item(row, 1)->text();
    QString s2 = table->item(row, 2)->text();
    Qt::CheckState checkState = table->item(row, 0)->checkState();
    table->item(row, 0)->setText(table->item(row-1, 0)->text());
    table->item(row, 1)->setText(table->item(row-1, 1)->text());
    table->item(row, 2)->setText(table->item(row-1, 2)->text());
    table->item(row, 0)->setCheckState( table->item(row-1, 0)->checkState() );
    table->item(row-1, 0)->setText(s0);
    table->item(row-1, 1)->setText(s1);
    table->item(row-1, 2)->setText(s2);
    table->item(row-1, 0)->setCheckState( checkState );
    table->setCurrentItem(table->item(row-1, 0));
}

void ChannelsImpl::on_down_clicked()
{
    int row = table->currentRow();
    if ( row > table->rowCount()-2 || row == -1 )
        return;
    QString s0 = table->item(row, 0)->text();
    QString s1 = table->item(row, 1)->text();
    QString s2 = table->item(row, 2)->text();
    Qt::CheckState checkState = table->item(row, 0)->checkState();
    table->item(row, 0)->setText(table->item(row+1, 0)->text());
    table->item(row, 1)->setText(table->item(row+1, 1)->text());
    table->item(row, 2)->setText(table->item(row+1, 2)->text());
    table->item(row, 0)->setCheckState( table->item(row+1, 0)->checkState() );
    table->item(row+1, 0)->setText(s0);
    table->item(row+1, 1)->setText(s1);
    table->item(row+1, 2)->setText(s2);
    table->item(row+1, 0)->setCheckState( checkState );
    table->setCurrentItem(table->item(row+1, 0));
}


void ChannelsImpl::on_selectAll_clicked()
{
    for (int row=0; row<table->rowCount(); row++ )
    {
        QTableWidgetItem *item = table->item(row, 0);
        item->setCheckState( Qt::Checked );
    }
}

void ChannelsImpl::on_unselectAll_clicked()
{
    for (int row=0; row<table->rowCount(); row++ )
    {
        QTableWidgetItem *item = table->item(row, 0);
        item->setCheckState( Qt::Unchecked );
    }
}


void ChannelsImpl::on_changeIcon_clicked()
{
    int row = table->currentRow();
    if ( row > table->rowCount()-2 || row == -1 )
        return;
    QTableWidgetItem *item = table->item(row, 0);
    QTableWidgetItem *item1 = table->item(row, 1);
    ChangeThumbImpl thumb(m_main, PairIcon(item1->text(), QPixmap()), item->text(), true);
    //QObject::connect(
        //&thumb,
        //SIGNAL(imageAvailable(PairIcon)),
        //this,
        //SLOT(slotImageAvailable(PairIcon))
    //);
    if ( thumb.exec() == QDialog::Accepted )
    {
        QPixmap pix = m_handler->pairIcon(item1->text()).pixmap();
        if( !pix.isNull() ){
            item->setIcon(QIcon(pix));
        }
    }
}
