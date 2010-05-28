#ifndef CHANGEICONIMPL_H
#define CHANGEICONIMPL_H
//
#include <QDialog>
#include "defs.h"
#include "ui_changeicon.h"
//

class ChannelIconItem;
class ChangeIconImpl : public QDialog, public Ui::ChangeIcon
{
Q_OBJECT
public:
	void deleteIcon(ChannelIconItem *item);
	ChangeIconImpl( QWidget * parent, QString filename, QString channelName );
	void channelIconClicked(ChannelIconItem *item, bool doubleClick);
private slots:
	void on_addButton_clicked();
	void on_filterEdit_textChanged(QString );
	void on_buttonBox_accepted();
private:
	QList<PairIcon> m_list;
	QString m_filename;
	QString m_selectedFilename;
	QString m_channelName;
	int m_x;
	int m_y;
};
#endif





