#ifndef CANAUXIMPL_H
#define CANAUXIMPL_H
//
#include <QDialog>
#include "xmldefaulthandler.h"
#include "ui_channels.h"
//
class ChannelsImpl : public QDialog, public Ui::Channels
{
Q_OBJECT
public:
	ChannelsImpl( QWidget * parent, QList<TvChannel> channels );
private slots:
	void on_up_clicked();
	void on_down_clicked();
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
private:
};
#endif






