#include "canauximpl.h"
#include "mainwindowimpl.h"
#include <QHeaderView>
#include <QSettings>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
//
CanauxImpl::CanauxImpl( QWidget * parent,QList<ChaineTV> chaines) 
	: QDialog(parent)
{
	setupUi(this);
    QSettings settings(MainWindowImpl::cheminIni() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
	QHeaderView *header = table->horizontalHeader();
	header->resizeSection( 0, 160 );
	header->resizeSection( 1, 160 );
	header->resizeSection( 2, 380 );
	table->verticalHeader()->hide();
	int row = 0;
	foreach(ChaineTV chaine, chaines)
	{
		QTableWidgetItem *newItem;
		table->setRowCount(row+1);
		newItem = new QTableWidgetItem( chaine.name.at(0).toUpper()+chaine.name.mid(1) );
		newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
		newItem->setCheckState( (Qt::CheckState)settings.value(chaine.id+"-isEnabled", Qt::Checked).toInt() );		
		table->setItem(row, 0, newItem);

		newItem = new QTableWidgetItem(chaine.id);
		newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
		table->setItem(row, 1, newItem);
		
		table->setItem(row++, 2, new QTableWidgetItem( settings.value(chaine.id, "rtsp://mafreebox.freebox.fr/fbxtv_pub/stream?namespace=1&service=NONE&flavour=sd").toString() ));
	}
    settings.endGroup();
}
//

void CanauxImpl::on_buttonBox_accepted()
{
    QSettings settings(MainWindowImpl::cheminIni() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
	for(int row=0; row<table->rowCount(); row++ )
	{
		QTableWidgetItem *item = table->item(row, 1);
		QTableWidgetItem *item2 = table->item(row, 2);
	    settings.setValue(item->text(), item2->text());
	    settings.setValue(item->text()+"-isEnabled", table->item(row, 0)->checkState());
	}
    settings.endGroup();
	close();
}

void CanauxImpl::on_buttonBox_rejected()
{
	close();
}

